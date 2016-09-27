
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include "main/php.h"
#include "main/php_ini.h"
#include "main/SAPI.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "ext/standard/url.h"
#include "ext/spl/spl_exceptions.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_portability.h"
#include "Zend/zend_smart_str.h"

#include "php_request.h"

extern void php_request_parse_accept(zval *return_value, const unsigned char *str, size_t len);
extern void php_request_parse_content_type(zval *return_value, const unsigned char *str, size_t len);
extern void php_request_parse_digest_auth(zval *return_value, const unsigned char *str, size_t len);

zend_class_entry * StdRequest_ce_ptr;
static zend_object_handlers StdRequest_obj_handlers;
static HashTable StdRequest_prop_handlers;

struct php_request_obj {
    zend_bool locked;
    zend_object std;
};

struct prop_handlers {
    zend_object_has_property_t has_property;
    zend_object_read_property_t read_property;
    zend_object_write_property_t write_property;
    zend_object_unset_property_t unset_property;
};

/* {{{ Argument Info */
ZEND_BEGIN_ARG_INFO_EX(StdRequest_construct_args, 0, 0, 0)
    ZEND_ARG_TYPE_INFO(0, globals, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(StdRequest_parseAccept_args, IS_ARRAY, NULL, 1)
    ZEND_ARG_TYPE_INFO(0, header, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(StdRequest_parseContentType_args, IS_ARRAY, NULL, 1)
    ZEND_ARG_TYPE_INFO(0, header, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(StdRequest_parseDigestAuth_args, IS_ARRAY, NULL, 1)
    ZEND_ARG_TYPE_INFO(0, header, IS_STRING, 0)
ZEND_END_ARG_INFO()
/* }}} Argument Info */

/* {{ php_request_detect_method */
static zend_bool php_request_detect_method(zval *return_value, zval *server)
{
    zend_string *tmp;
    zval *val;
    zend_bool xhr = 0;
    zend_string *method;

    // determine method from request
    val = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("REQUEST_METHOD"));
    if( !val || Z_TYPE_P(val) != IS_STRING ) {
        return xhr;
    }
    method = Z_STR_P(val);

    // XmlHttpRequest method override?
    if( zend_string_equals_literal_ci(method, "POST") ) {
        val = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_X_HTTP_METHOD_OVERRIDE"));
        if( val && Z_TYPE_P(val) == IS_STRING ) {
            method = Z_STR_P(val);
            xhr = 1;
        }
    }

    if( method ) {
        tmp = zend_string_dup(method, 0);
        php_strtoupper(ZSTR_VAL(tmp), ZSTR_LEN(tmp));
        ZVAL_STR(return_value, tmp);
    }

    return xhr;
}
/* }}} */

/* {{{ php_request_is_secure */
static zend_bool php_request_is_secure(zval *server)
{
    zval *tmp;

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTPS"))) &&
        Z_TYPE_P(tmp) == IS_STRING &&
        zend_string_equals_literal_ci(Z_STR_P(tmp), "on") ) {
        return 1;
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("SERVER_PORT"))) &&
        443 == zval_get_long(tmp) ) {
        return 1;
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_X_FORWARDED_PROTO"))) &&
        Z_TYPE_P(tmp) == IS_STRING &&
        zend_string_equals_literal_ci(Z_STR_P(tmp), "https") ) {
        return 1;
    }

    return 0;
}
/* }}} */

/* {{{ php_request_detect_url */
static inline const unsigned char *extract_port_from_host(const unsigned char *host, size_t len)
{
    const unsigned char *right = host + len - 1;
    const unsigned char *left = len > 6 ? right - 6 : host;
    const unsigned char *pos = right;
    for( ; pos > left; pos-- ) {
        if( !isdigit(*pos) ) {
            if( *pos == ':' ) {
                return pos + 1;
            }
            break;
        }
    }
    return NULL;
}

static inline zend_string *extract_host_from_server(zval *server)
{
    zval *tmp;
    zend_string *host;

    // Get host
    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_HOST"))) &&
        Z_TYPE_P(tmp) == IS_STRING ) {
        host = Z_STR_P(tmp);
    } else if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("SERVER_NAME"))) &&
               Z_TYPE_P(tmp) == IS_STRING ) {
        host = Z_STR_P(tmp);
    } else {
        host = NULL;
    }

    return host;
}

static inline zend_long extract_port_from_server(zval *server, zend_string * host)
{
    zval *tmp;

    // Get port
    if( NULL != extract_port_from_host(ZSTR_VAL(host), ZSTR_LEN(host)) ) {
        // no need to extract
    } else if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("SERVER_PORT"))) ) {
        return zval_get_long(tmp);
    }

    return 0;
}

static inline zend_string *extract_uri_from_server(zval *server)
{
    zval *tmp;

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("REQUEST_URI"))) &&
        Z_TYPE_P(tmp) == IS_STRING ) {
        return Z_STR_P(tmp);
    }

    return NULL;
}

static zend_string *php_request_detect_url(zval *server)
{
    zval *tmp;
    zend_string *host;
    zend_long port;
    zend_string *uri;
    smart_str buf = {0};
    zend_bool is_secure = php_request_is_secure(server);

    // Get host
    if( !(host = extract_host_from_server(server)) ) {
        return NULL;
    }

    port = extract_port_from_server(server, host);

    // Get uri
    uri = extract_uri_from_server(server);

    // Form URL
    smart_str_alloc(&buf, 1024, 0);
    if( is_secure ) {
        smart_str_appendl_ex(&buf, ZEND_STRL("https://"), 0);
    } else {
        smart_str_appendl_ex(&buf, ZEND_STRL("http://"), 0);
    }
    smart_str_append_ex(&buf, host, 0);
    if( port > 0 ) {
        smart_str_appendc_ex(&buf, ':', 0);
        smart_str_append_long(&buf, port);
    }
    if( uri ) {
        smart_str_append_ex(&buf, uri, 0);
    }
    smart_str_0(&buf);

    return buf.s;
}
/* }}} */

/* {{{ php_request_normalize_headers */
static const char http_str[] = "HTTP_";
static const size_t http_len = sizeof(http_str) - 1;

static void php_request_normalize_headers(zval *return_value, zval *server)
{
    zend_string *key;
    zend_ulong index;
    zval *val;
    zend_string *tmp;

    array_init(return_value);

    // Main headers
    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(server), index, key, val) {
        if( key && ZSTR_LEN(key) > 5 && strncmp(ZSTR_VAL(key), http_str, http_len) == 0 ) {
            tmp = zend_string_init(ZSTR_VAL(key) + http_len, ZSTR_LEN(key) - http_len, 0);
            php_request_normalize_header_name(ZSTR_VAL(tmp), ZSTR_LEN(tmp));
            add_assoc_zval_ex(return_value, ZSTR_VAL(tmp), ZSTR_LEN(tmp), val);
            zend_string_release(tmp);
        }
    } ZEND_HASH_FOREACH_END();

    // RFC 3875 headers not prefixed with HTTP_*
    if( val = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("CONTENT_LENGTH")) ) {
        add_assoc_zval_ex(return_value, ZEND_STRL("Content-Length"), val);
    }
    if( val = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("CONTENT_TYPE")) ) {
        add_assoc_zval_ex(return_value, ZEND_STRL("Content-Type"), val);
    }
}
/* }}} */

/* {{{ php_request_normalize_files */
static void php_request_upload_from_spec(zval *return_value, zval *file);

static inline void php_request_copy_upload_key(zval *return_value, zval *nested, const char *key1, size_t key1_len, zend_ulong index2, zend_string *key2)
{
    zval *tmp = zend_hash_str_find(Z_ARRVAL_P(nested), key1, key1_len);
    if( !tmp ) {
        goto err;
    }

    if( Z_TYPE_P(tmp) == IS_ARRAY ) {
        if( key2 ) {
            tmp = zend_hash_find(Z_ARRVAL_P(tmp), key2);
        } else {
            tmp = zend_hash_index_find(Z_ARRVAL_P(tmp), index2);
        }
    }

    if( tmp ) {
        add_assoc_zval_ex(return_value, key1, key1_len, tmp);
    } else {
        err:
        add_assoc_null_ex(return_value, key1, key1_len);
    }
}

static inline void php_request_upload_from_nested(zval *return_value, zval *nested, zval *tmp_name)
{
    zend_string *key;
    zend_ulong index;
    zval tmp = {0};
    zval tmp2 = {0};

    array_init(return_value);

    ZEND_HASH_FOREACH_KEY(Z_ARRVAL_P(tmp_name), index, key) {
        array_init(&tmp);
        php_request_copy_upload_key(&tmp, nested, ZEND_STRL("error"), index, key);
        php_request_copy_upload_key(&tmp, nested, ZEND_STRL("name"), index, key);
        php_request_copy_upload_key(&tmp, nested, ZEND_STRL("size"), index, key);
        php_request_copy_upload_key(&tmp, nested, ZEND_STRL("tmp_name"), index, key);
        php_request_copy_upload_key(&tmp, nested, ZEND_STRL("type"), index, key);

        php_request_upload_from_spec(&tmp2, &tmp);
        if( key ) {
            add_assoc_zval_ex(return_value, ZSTR_VAL(key), ZSTR_LEN(key), &tmp2);
        } else {
            add_index_zval(return_value, index, &tmp2);
        }
    } ZEND_HASH_FOREACH_END();
}

static void php_request_upload_from_spec(zval *return_value, zval *file)
{
    zval *tmp = zend_hash_str_find(Z_ARRVAL_P(file), ZEND_STRL("tmp_name"));
    if( tmp && Z_TYPE_P(tmp) == IS_ARRAY ) {
        php_request_upload_from_nested(return_value, file, tmp);
    } else {
        ZVAL_ZVAL(return_value, file, 0, 0);
    }
}

void php_request_normalize_files(zval *return_value, zval *files)
{
    zend_string *key;
    zend_ulong index;
    zval *val;
    zval tmp = {0};

    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(files), index, key, val) {
        if( Z_TYPE_P(val) == IS_ARRAY ) {
            php_request_upload_from_spec(&tmp, val);
            if( key ) {
                add_assoc_zval_ex(return_value, ZSTR_VAL(key), ZSTR_LEN(key), &tmp);
            } else {
                add_index_zval(return_value, index, &tmp);
            }
        }
    } ZEND_HASH_FOREACH_END();
}
/* }}} */

/* {{{ Z_REQUEST_P */
static inline struct php_request_obj *php_request_fetch_object(zend_object *obj)
{
    return (struct php_request_obj *)((char*)(obj) - XtOffsetOf(struct php_request_obj, std));
}
#define Z_REQUEST_P(zv) php_request_fetch_object(Z_OBJ_P((zv)))
/* }}} */

/* {{{ php_request_obj_create */
static zend_object *php_request_obj_create(zend_class_entry *ce)
{
    struct php_request_obj *obj;

    obj = ecalloc(1, sizeof(*obj) + zend_object_properties_size(ce));
    zend_object_std_init(&obj->std, ce);
    object_properties_init(&obj->std, ce);
    obj->std.handlers = &StdRequest_obj_handlers;

    obj->locked = 1;

    return &obj->std;
}
/* }}} */

/* {{{ php_request_object_default_has_property */
static int php_request_object_default_has_property(zval *object, zval *member, int has_set_exists, void **cache_slot)
{
    return 1;
}
/* }}} */

/* {{{ php_request_throw_readonly_exception */
static inline void php_request_throw_readonly_exception(zval *object, zval *member)
{
    zend_string *ce_name = Z_OBJCE_P(object)->name;
    zend_string *member_str = zval_get_string(member);
    zend_throw_exception_ex(spl_ce_RuntimeException, 0, "%.*s::$%.*s is read-only.", ZSTR_LEN(ce_name), ZSTR_VAL(ce_name), ZSTR_LEN(member_str), ZSTR_VAL(member_str));
    zend_string_release(member_str);
}
/* }}} */

/* {{{ php_request_object_default_read_property */
static zval *php_request_object_default_read_property(zval *object, zval *member, int type, void **cache_slot, zval *rv)
{
    zval *retval;
    ZVAL_UNDEF(rv);
    retval = std_object_handlers.read_property(object, member, type, cache_slot, rv);
    // Make sure the property can't be modified
    if( !Z_ISREF_P(rv) && (type == BP_VAR_W || type == BP_VAR_RW  || type == BP_VAR_UNSET) ) {
        SEPARATE_ZVAL(rv);
        php_request_throw_readonly_exception(object, member);
    }
    return retval;
}
/* }}} */

/* {{{ php_request_object_default_write_property */
static void php_request_object_default_write_property(zval *object, zval *member, zval *value, void **cache_slot)
{
    struct php_request_obj *intern = Z_REQUEST_P(object);
    if( intern->locked ) {
        php_request_throw_readonly_exception(object, member);
    } else {
        std_object_handlers.write_property(object, member, value, cache_slot);
    }
}
/* }}} */

/* {{{ php_request_object_default_unset_property */
static void php_request_object_default_unset_property(zval *object, zval *member, void **cache_slot)
{
    struct php_request_obj *intern = Z_REQUEST_P(object);
    if( intern->locked ) {
        php_request_throw_readonly_exception(object, member);
    } else {
        std_object_handlers.unset_property(object, member, cache_slot);
    }
}
/* }}} */

/* {{{ php_request_object_content_read_property */
static zval *php_request_object_content_read_property(zval *object, zval *member, int type, void **cache_slot, zval *rv)
{
    php_stream *stream;
    zend_string *str;

    if( (type == BP_VAR_W || type == BP_VAR_RW  || type == BP_VAR_UNSET) ) {
        php_request_throw_readonly_exception(object, member);
        return rv;
    }

    ZVAL_NULL(rv);
    stream = php_stream_open_wrapper_ex("php://input", "rb", REPORT_ERRORS, NULL, NULL);
    if( stream ) {
        if ((str = php_stream_copy_to_mem(stream, PHP_STREAM_COPY_ALL, 0))) {
            ZVAL_STR(rv, str);
        }
        php_stream_close(stream);
    }

    return rv;
}
/* }}} */

/* {{{ php_request_object_has_property */
static int php_request_object_has_property(zval *object, zval *member, int has_set_exists, void **cache_slot)
{
//    if( !Z_OBJCE_P(object)->__isset && !std_object_handlers.has_property(object, member, has_set_exists, cache_slot) ) {
//        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "StdRequest::$%.*s does not exist.", Z_STRLEN_P(member), Z_STRVAL_P(member));
//        return 0;
//    }
    struct prop_handlers *hnd = zend_hash_str_find_ptr(&StdRequest_prop_handlers, Z_STRVAL_P(member), Z_STRLEN_P(member));
    return (hnd ? hnd->has_property : std_object_handlers.has_property)(object, member, has_set_exists, cache_slot);
}
/* }}} */

/* {{{ php_request_object_read_property */
static zval *php_request_object_read_property(zval *object, zval *member, int type, void **cache_slot, zval *rv)
{
    if( !Z_OBJCE_P(object)->__get && !std_object_handlers.has_property(object, member, 2, cache_slot) ) {
        zend_string *ce_name = Z_OBJCE_P(object)->name;
        zend_string *member_str = zval_get_string(member);
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "%.*s::$%.*s does not exist.", ZSTR_LEN(ce_name), ZSTR_VAL(ce_name), ZSTR_LEN(member_str), ZSTR_VAL(member_str));
        zend_string_release(member_str);
        return rv;
    }
    struct prop_handlers *hnd = zend_hash_str_find_ptr(&StdRequest_prop_handlers, Z_STRVAL_P(member), Z_STRLEN_P(member));
    return (hnd ? hnd->read_property : std_object_handlers.read_property)(object, member, type, cache_slot, rv);
}
/* }}} */

/* {{{ php_request_object_write_property */
static void php_request_object_write_property(zval *object, zval *member, zval *value, void **cache_slot)
{
    if( !Z_OBJCE_P(object)->__set && !std_object_handlers.has_property(object, member, 2, cache_slot) ) {
        php_request_throw_readonly_exception(object, member);
        return;
    }
    struct prop_handlers *hnd = zend_hash_str_find_ptr(&StdRequest_prop_handlers, Z_STRVAL_P(member), Z_STRLEN_P(member));
    return (hnd ? hnd->write_property : std_object_handlers.write_property)(object, member, value, cache_slot);
}
/* }}} */

/* {{{ php_request_object_unset_property */
static void php_request_object_unset_property(zval *object, zval *member, void **cache_slot)
{
    struct prop_handlers *hnd = zend_hash_str_find_ptr(&StdRequest_prop_handlers, Z_STRVAL_P(member), Z_STRLEN_P(member));
    return (hnd ? hnd->unset_property : std_object_handlers.unset_property)(object, member, cache_slot);
}
/* }}} */

/* {{{ register_prop_handlers */
static inline void register_prop_handlers(
    const char *name,
    size_t name_length,
    zend_object_has_property_t has_property,
    zend_object_read_property_t read_property,
    zend_object_write_property_t write_property,
    zend_object_unset_property_t unset_property
) {
    struct prop_handlers hnd = {0};
    hnd.has_property = has_property ?: std_object_handlers.has_property;
    hnd.read_property = read_property ?: std_object_handlers.read_property;
    hnd.write_property = write_property ?: std_object_handlers.write_property;
    hnd.unset_property = unset_property ?: std_object_handlers.unset_property;
    zend_hash_str_update_mem(&StdRequest_prop_handlers, name, name_length, &hnd, sizeof(hnd));
}
static inline void register_default_prop_handlers(const char *name, size_t name_length)
{
    register_prop_handlers(
        name,
        name_length,
        php_request_object_default_has_property,
        php_request_object_default_read_property,
        php_request_object_default_write_property,
        php_request_object_default_unset_property
    );
}
/* }}} */

/* {{{ proto StdRequest::__construct([ array $globals ]) */
static inline void php_request_copy_global(
    zval *obj,
    const char *obj_key,
    size_t obj_key_length,
    zval *globals,
    const char *glob_key,
    size_t glob_key_length
) {
    zval *tmp = NULL;
    if( globals && Z_TYPE_P(globals) == IS_ARRAY ) {
        tmp = zend_hash_str_find(Z_ARRVAL_P(globals), glob_key, glob_key_length);
    }
    if( !tmp ) {
        if( PG(auto_globals_jit) ) {
            zend_is_auto_global_str(glob_key, glob_key_length);
        }
        tmp = zend_hash_str_find(&EG(symbol_table), glob_key, glob_key_length);
    }
    if( tmp ) {
        zend_update_property(Z_OBJCE_P(obj), obj, obj_key, obj_key_length, tmp);
        Z_TRY_ADDREF_P(tmp);
    }
}
#define php_request_copy_global_lit(obj, obj_key, glob, glob_key) php_request_copy_global(obj, ZEND_STRL(obj_key), glob, ZEND_STRL(glob_key))

static inline void php_request_set_url(zval *object, zval *server)
{
    zend_string *tmp;
    php_url *url;
    zval arr = {0};

    tmp = php_request_detect_url(server);
    if( !tmp ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "Could not determine host for StdRequest.");
        return;
    }

    url = php_url_parse_ex(ZSTR_VAL(tmp), ZSTR_LEN(tmp));
    zend_string_release(tmp);
    if( !url ) {
        return;
    }

    // Form array
    array_init_size(&arr, 8);
    if( url->scheme ) {
        add_assoc_string(&arr, "scheme", url->scheme);
    } else {
        add_assoc_null(&arr, "scheme");
    }
    if( url->host ) {
        add_assoc_string(&arr, "host", url->host);
    } else {
        add_assoc_null(&arr, "host");
    }
    if( url->port ) {
        add_assoc_long(&arr, "port", url->port);
    } else {
        add_assoc_null(&arr, "port");
    }
    if( url->user ) {
        add_assoc_string(&arr, "user", url->user);
    } else {
        add_assoc_null(&arr, "user");
    }
    if( url->pass ) {
        add_assoc_string(&arr, "pass", url->pass);
    } else {
        add_assoc_null(&arr, "pass");
    }
    if( url->path ) {
        add_assoc_string(&arr, "path", url->path);
    } else {
        add_assoc_null(&arr, "path");
    }
    if( url->query ) {
        add_assoc_string(&arr, "query", url->query);
    } else {
        add_assoc_null(&arr, "query");
    }
    if( url->fragment ) {
        add_assoc_string(&arr, "fragment", url->fragment);
    } else {
        add_assoc_null(&arr, "fragment");
    }

    zend_update_property(Z_OBJCE_P(object), object, ZEND_STRL("url"), &arr);

    php_url_free(url);
}

static inline void php_request_set_accept_by_name(zval *object, zval *server, const char *src, size_t src_length, const char *dest, size_t dest_length)
{
    zval val;
    zval *tmp;

    array_init(&val);
    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), src, src_length)) ) {
        php_request_parse_accept(&val, Z_STRVAL_P(tmp), Z_STRLEN_P(tmp));
    }
    zend_update_property(Z_OBJCE_P(object), object, dest, dest_length, &val);
}

static inline void php_request_parse_accept_language(zval *lang)
{
    zend_string *key;
    zend_ulong index;
    zval *val;
    zval *value;
    zval rv;
    zend_string *str;
    char *r1;
    char *r2;
    zval type = {0};
    zval subtype = {0};

    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(lang), val) {
        value = zend_hash_str_find(Z_ARRVAL_P(val), ZEND_STRL("value"));
        if( value ) {
            str = zval_get_string(value);
            r1 = ZSTR_VAL(str);
            r2 = memchr(r1, '-', ZSTR_LEN(str));
            if( r2 ) {
                ZVAL_STRINGL(&type, r1, r2 - r1);
                ZVAL_STRINGL(&subtype, r2 + 1, r1 + ZSTR_LEN(str) - r2 - 1);
            } else {
                ZVAL_STR(&type, str);
                ZVAL_NULL(&subtype);
            }
            add_assoc_zval_ex(val, ZEND_STRL("type"), &type);
            add_assoc_zval_ex(val, ZEND_STRL("subtype"), &subtype);
        }
    } ZEND_HASH_FOREACH_END();
}

static inline void php_request_set_accept_language(zval *object, zval *server)
{
    zval val;
    zval *tmp;

    array_init(&val);
    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_ACCEPT_LANGUAGE"))) ) {
        php_request_parse_accept(&val, Z_STRVAL_P(tmp), Z_STRLEN_P(tmp));
        php_request_parse_accept_language(&val);
    }
    zend_update_property(Z_OBJCE_P(object), object, ZEND_STRL("acceptLanguage"), &val);
}

static inline void php_request_set_auth(zval *object, zval *server)
{
    zval *tmp;
    zval digest = {0};

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("PHP_AUTH_PW"))) ) {
        zend_update_property(Z_OBJCE_P(object), object, ZEND_STRL("authPw"), tmp);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("PHP_AUTH_TYPE"))) ) {
        zend_update_property(Z_OBJCE_P(object), object, ZEND_STRL("authType"), tmp);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("PHP_AUTH_USER"))) ) {
        zend_update_property(Z_OBJCE_P(object), object, ZEND_STRL("authUser"), tmp);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("PHP_AUTH_DIGEST"))) ) {
        zend_string *str = zval_get_string(tmp);
        php_request_parse_digest_auth(&digest, ZSTR_VAL(str), ZSTR_LEN(str));
        zend_update_property(Z_OBJCE_P(object), object, ZEND_STRL("authDigest"), &digest);
    }

}

static inline void php_request_set_content(zval *object, zval *server)
{
    zval *tmp;
    zval zv = {0};
    zval contentType = {0};

    // content body read moved to prop handler

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_CONTENT_MD5"))) ) {
        zend_update_property(Z_OBJCE_P(object), object, ZEND_STRL("contentMd5"), tmp);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_CONTENT_LENGTH"))) ) {
        zend_update_property(Z_OBJCE_P(object), object, ZEND_STRL("contentLength"), tmp);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_CONTENT_TYPE"))) && Z_TYPE_P(tmp) == IS_STRING ) {
        php_request_parse_content_type(&contentType, Z_STRVAL_P(tmp), Z_STRLEN_P(tmp));
        if( Z_TYPE(contentType) == IS_ARRAY ) {
            // contentType
            tmp = zend_hash_str_find(Z_ARRVAL(contentType), ZEND_STRL("value"));
            if( tmp ) {
                zend_update_property(Z_OBJCE_P(object), object, ZEND_STRL("contentType"), tmp);
            }
            // charset
            tmp = zend_hash_str_find(Z_ARRVAL(contentType), ZEND_STRL("charset"));
            if( tmp ) {
                zend_update_property(Z_OBJCE_P(object), object, ZEND_STRL("contentCharset"), tmp);
            }
        }
    }
}

PHP_METHOD(StdRequest, __construct)
{
    zval *_this_zval;
    zval *zv_globals = NULL;
    struct php_request_obj *intern;
    zval *server;
    zval *files;
    zval rv = {0};
    zval method = {0};
    zval headers = {0};
    zval uploads = {0};

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_ZVAL(zv_globals)
    ZEND_PARSE_PARAMETERS_END();

    _this_zval = getThis();
    intern = Z_REQUEST_P(_this_zval);

    // Unlock
    intern->locked = 0;

    // Copy superglobals
    php_request_copy_global_lit(_this_zval, "env", zv_globals, "_ENV");
    php_request_copy_global_lit(_this_zval, "server", zv_globals, "_SERVER");
    php_request_copy_global_lit(_this_zval, "cookie", zv_globals, "_COOKIE");
    php_request_copy_global_lit(_this_zval, "files", zv_globals, "_FILES");
    php_request_copy_global_lit(_this_zval, "get", zv_globals, "_GET");
    php_request_copy_global_lit(_this_zval, "post", zv_globals, "_POST");

    // Read back server property
    server = zend_read_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("server"), 0, &rv);

    // Internal setters that require server
    if( server && Z_TYPE_P(server) == IS_ARRAY ) {
        // method
        ZVAL_STRING(&method, "");
        zend_bool xhr = php_request_detect_method(&method, server);
        zend_update_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("method"), &method);
        zend_update_property_bool(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("xhr"), xhr);

        // secure
        zend_bool secure = php_request_is_secure(server);
        zend_update_property_bool(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("secure"), secure);

        // headers
        php_request_normalize_headers(&headers, server);
        zend_update_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("headers"), &headers);

        // url
        php_request_set_url(_this_zval, server);

        // accepts
        php_request_set_accept_by_name(_this_zval, server, ZEND_STRL("HTTP_ACCEPT"), ZEND_STRL("acceptMedia"));
        php_request_set_accept_by_name(_this_zval, server, ZEND_STRL("HTTP_ACCEPT_CHARSET"), ZEND_STRL("acceptCharset"));
        php_request_set_accept_by_name(_this_zval, server, ZEND_STRL("HTTP_ACCEPT_ENCODING"), ZEND_STRL("acceptEncoding"));
        php_request_set_accept_language(_this_zval, server);

        // auth
        php_request_set_auth(_this_zval, server);
        php_request_set_content(_this_zval, server);
    }

    // Read back files property
    files = zend_read_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("files"), 0, &rv);

    if( files && Z_TYPE_P(files) == IS_ARRAY ) {
        array_init(&uploads);
        php_request_normalize_files(&uploads, files);
        zend_update_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("uploads"), &uploads);
    }

    // Lock the object
    intern->locked = 1;
}
/* }}} StdRequest::__construct */

/* {{{ proto array StdRequest::parseAccept(string $header) */
PHP_METHOD(StdRequest, parseAccept)
{
    zend_string *header;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(header)
    ZEND_PARSE_PARAMETERS_END();

    //array_init(return_value);
    php_request_parse_accept(return_value, ZSTR_VAL(header), ZSTR_LEN(header));

}
/* }}} StdRequest::parseAccept */

/* {{{ proto array StdRequest::parseContentType(string $header) */
PHP_METHOD(StdRequest, parseContentType)
{
    zend_string *header;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(header)
    ZEND_PARSE_PARAMETERS_END();

    php_request_parse_content_type(return_value, ZSTR_VAL(header), ZSTR_LEN(header));
}
/* }}} StdRequest::parseContentType */

/* {{{ proto array StdRequest::parseDigestAuth(string $header) */
PHP_METHOD(StdRequest, parseDigestAuth)
{
    zend_string *header;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(header)
    ZEND_PARSE_PARAMETERS_END();

    php_request_parse_digest_auth(return_value, ZSTR_VAL(header), ZSTR_LEN(header));
}
/* }}} StdRequest::parseDigestAuth */

/* {{{ StdRequest methods */
static zend_function_entry StdRequest_methods[] = {
    PHP_ME(StdRequest, __construct, StdRequest_construct_args, ZEND_ACC_PUBLIC)
    PHP_ME(StdRequest, parseAccept, StdRequest_parseAccept_args, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(StdRequest, parseContentType, StdRequest_parseContentType_args, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(StdRequest, parseDigestAuth, StdRequest_parseDigestAuth_args, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_FE_END
};
/* }}} StdRequest methods */

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(stdrequest)
{
    zend_class_entry ce;

    zend_hash_init(&StdRequest_prop_handlers, 0, NULL, NULL, 1);

    memcpy(&StdRequest_obj_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    StdRequest_obj_handlers.offset = XtOffsetOf(struct php_request_obj, std);
    StdRequest_obj_handlers.has_property = php_request_object_has_property;
    StdRequest_obj_handlers.read_property = php_request_object_read_property;
    StdRequest_obj_handlers.write_property = php_request_object_write_property;
    StdRequest_obj_handlers.unset_property = php_request_object_unset_property;
    StdRequest_obj_handlers.get_property_ptr_ptr = NULL;

    INIT_CLASS_ENTRY(ce, "StdRequest", StdRequest_methods);
    StdRequest_ce_ptr = zend_register_internal_class(&ce);
    StdRequest_ce_ptr->create_object = php_request_obj_create;

    zend_declare_property_null(StdRequest_ce_ptr, ZEND_STRL("acceptCharset"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("acceptCharset"));
    zend_declare_property_null(StdRequest_ce_ptr, ZEND_STRL("acceptEncoding"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("acceptEncoding"));
    zend_declare_property_null(StdRequest_ce_ptr, ZEND_STRL("acceptLanguage"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("acceptLanguage"));
    zend_declare_property_null(StdRequest_ce_ptr, ZEND_STRL("acceptMedia"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("acceptMedia"));
    zend_declare_property_null(StdRequest_ce_ptr, ZEND_STRL("authDigest"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("authDigest"));
    zend_declare_property_null(StdRequest_ce_ptr, ZEND_STRL("authPw"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("authPw"));
    zend_declare_property_null(StdRequest_ce_ptr, ZEND_STRL("authType"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("authType"));
    zend_declare_property_null(StdRequest_ce_ptr, ZEND_STRL("authUser"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("authUser"));
    zend_declare_property_null(StdRequest_ce_ptr, ZEND_STRL("content"), ZEND_ACC_PUBLIC);
    register_prop_handlers(
        ZEND_STRL("content"),
        php_request_object_default_has_property,
        php_request_object_content_read_property,
        php_request_object_default_write_property,
        php_request_object_default_unset_property
    );
    zend_declare_property_null(StdRequest_ce_ptr, ZEND_STRL("contentCharset"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("contentCharset"));
    zend_declare_property_null(StdRequest_ce_ptr, ZEND_STRL("contentLength"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("contentLength"));
    zend_declare_property_null(StdRequest_ce_ptr, ZEND_STRL("contentMd5"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("contentMd5"));
    zend_declare_property_null(StdRequest_ce_ptr, ZEND_STRL("contentType"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("contentType"));
    zend_declare_property_null(StdRequest_ce_ptr, ZEND_STRL("cookie"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("cookie"));
    zend_declare_property_null(StdRequest_ce_ptr, ZEND_STRL("env"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("env"));
    zend_declare_property_null(StdRequest_ce_ptr, ZEND_STRL("files"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("files"));
    zend_declare_property_null(StdRequest_ce_ptr, ZEND_STRL("get"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("get"));
    zend_declare_property_null(StdRequest_ce_ptr, ZEND_STRL("headers"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("headers"));
    zend_declare_property_string(StdRequest_ce_ptr, ZEND_STRL("method"), "", ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("method"));
    zend_declare_property_null(StdRequest_ce_ptr, ZEND_STRL("post"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("post"));
    zend_declare_property_bool(StdRequest_ce_ptr, ZEND_STRL("secure"), 0, ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("secure"));
    zend_declare_property_null(StdRequest_ce_ptr, ZEND_STRL("server"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("server"));
    zend_declare_property_null(StdRequest_ce_ptr, ZEND_STRL("uploads"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("uploads"));
    zend_declare_property_null(StdRequest_ce_ptr, ZEND_STRL("url"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("url"));
    zend_declare_property_bool(StdRequest_ce_ptr, ZEND_STRL("xhr"), 0, ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("xhr"));

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION */
PHP_MSHUTDOWN_FUNCTION(stdrequest)
{
    zend_hash_destroy(&StdRequest_prop_handlers);

    return SUCCESS;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: et sw=4 ts=4
 */
