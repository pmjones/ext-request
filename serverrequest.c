
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

extern void server_request_parse_accept(zval *return_value, const unsigned char *str, size_t len);
extern void server_request_parse_content_type(zval *return_value, const unsigned char *str, size_t len);
extern void server_request_parse_digest_auth(zval *return_value, const unsigned char *str, size_t len);
extern void server_request_parse_x_forwarded_for(zval *return_value, const unsigned char *str, size_t len);
extern void server_request_parse_x_forwarded(zval *return_value, const unsigned char *str, size_t len);
extern void server_request_normalize_header_name(char *key, size_t key_length);

zend_class_entry *ServerRequest_ce_ptr;
static zend_object_handlers ServerRequest_obj_handlers;
static HashTable ServerRequest_prop_handlers;

struct prop_handlers {
    zend_object_has_property_t has_property;
    zend_object_read_property_t read_property;
    zend_object_write_property_t write_property;
    zend_object_unset_property_t unset_property;
};

static inline zend_class_entry *get_scope()
{
#if PHP_MINOR_VERSION >= 1
    if( EG(fake_scope) ) {
        return EG(fake_scope);
    } else {
        return zend_get_executed_scope();
    }
#else
    return EG(scope);
#endif
}

/* {{{ Argument Info */
ZEND_BEGIN_ARG_INFO_EX(ServerRequest_construct_args, 0, 0, 0)
    ZEND_ARG_TYPE_INFO(0, globals, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

REQUEST_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerRequest_parseAccept_args, 0, 1, IS_ARRAY, 1)
    ZEND_ARG_TYPE_INFO(0, header, IS_STRING, 0)
ZEND_END_ARG_INFO()

REQUEST_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerRequest_parseContentType_args, 0, 1, IS_ARRAY, 1)
    ZEND_ARG_TYPE_INFO(0, header, IS_STRING, 0)
ZEND_END_ARG_INFO()

REQUEST_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerRequest_parseDigestAuth_args, 0, 1, IS_ARRAY, 1)
    ZEND_ARG_TYPE_INFO(0, header, IS_STRING, 0)
ZEND_END_ARG_INFO()

REQUEST_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(ServerRequest_withInput_args, 0, 1, "ServerRequest", 0)
    ZEND_ARG_INFO(0, input)
ZEND_END_ARG_INFO()

REQUEST_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(ServerRequest_withParam_args, 0, 2, "ServerRequest", 0)
    ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
    ZEND_ARG_INFO(0, val)
ZEND_END_ARG_INFO()

REQUEST_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(ServerRequest_withParams_args, 0, 1, "ServerRequest", 0)
    ZEND_ARG_ARRAY_INFO(0, params, 0)
ZEND_END_ARG_INFO()

REQUEST_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(ServerRequest_withoutParam_args, 0, 1, "ServerRequest", 0)
    ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
ZEND_END_ARG_INFO()

REQUEST_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(ServerRequest_withoutParams_args, 0, 1, "ServerRequest", 0)
    ZEND_ARG_ARRAY_INFO(0, keys, 1)
ZEND_END_ARG_INFO()

REQUEST_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(ServerRequest_withUrl_args, 0, 1, "ServerRequest", 0)
    ZEND_ARG_ARRAY_INFO(0, url, 0)
ZEND_END_ARG_INFO()
/* }}} Argument Info */

/* {{ server_request_detect_method */
static zend_bool server_request_detect_method(zval *return_value, zval *server)
{
    zend_string *tmp;
    zval *val;
    zend_bool xhr = 0;
    zend_string *method;

    // determine xhr value from X-Requested-With header
    zval *xreqwith_val;
    zend_string *xreqwith_str;
    xreqwith_val = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_X_REQUESTED_WITH"));
    if( xreqwith_val && Z_TYPE_P(xreqwith_val) == IS_STRING ) {
        xreqwith_str = Z_STR_P(xreqwith_val);
        if( zend_string_equals_literal_ci(xreqwith_str, "XmlHttpRequest") ) {
            xhr = 1;
        }
    }

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

/* {{{ server_request_is_secure */
static zend_bool server_request_is_secure(zval *server)
{
    zval *tmp;

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTPS"))) &&
        Z_TYPE_P(tmp) == IS_STRING &&
        zend_string_equals_literal_ci(Z_STR_P(tmp), "on") ) {
        return 1;
    }

    return 0;
}
/* }}} */

/* {{{ server_request_detect_url */
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

static zend_string *server_request_detect_url(zval *server)
{
    zval *tmp;
    zend_string *host;
    zend_long port;
    zend_string *uri;
    smart_str buf = {0};
    zend_bool is_secure = server_request_is_secure(server);

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

/* {{{ server_request_normalize_headers */
static const char http_str[] = "HTTP_";
static const size_t http_len = sizeof(http_str) - 1;

static void server_request_normalize_headers(zval *return_value, zval *server)
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
            server_request_normalize_header_name(ZSTR_VAL(tmp), ZSTR_LEN(tmp));
            add_assoc_zval_ex(return_value, ZSTR_VAL(tmp), ZSTR_LEN(tmp), val);
            zend_string_release(tmp);
        }
    } ZEND_HASH_FOREACH_END();

    // RFC 3875 headers not prefixed with HTTP_*
    if( val = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("CONTENT_LENGTH")) ) {
        add_assoc_zval_ex(return_value, ZEND_STRL("content-length"), val);
    }
    if( val = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("CONTENT_TYPE")) ) {
        add_assoc_zval_ex(return_value, ZEND_STRL("content-type"), val);
    }
}
/* }}} */

/* {{{ server_request_normalize_files */
static void server_request_upload_from_spec(zval *return_value, zval *file);

static inline void server_request_copy_upload_key(zval *return_value, zval *nested, const char *key1, size_t key1_len, zend_ulong index2, zend_string *key2)
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

static inline void server_request_upload_from_nested(zval *return_value, zval *nested, zval *tmp_name)
{
    zend_string *key;
    zend_ulong index;
    zval tmp = {0};
    zval tmp2 = {0};

    array_init(return_value);

    ZEND_HASH_FOREACH_KEY(Z_ARRVAL_P(tmp_name), index, key) {
        array_init(&tmp);
        server_request_copy_upload_key(&tmp, nested, ZEND_STRL("error"), index, key);
        server_request_copy_upload_key(&tmp, nested, ZEND_STRL("name"), index, key);
        server_request_copy_upload_key(&tmp, nested, ZEND_STRL("size"), index, key);
        server_request_copy_upload_key(&tmp, nested, ZEND_STRL("tmp_name"), index, key);
        server_request_copy_upload_key(&tmp, nested, ZEND_STRL("type"), index, key);

        server_request_upload_from_spec(&tmp2, &tmp);
        if( key ) {
            add_assoc_zval_ex(return_value, ZSTR_VAL(key), ZSTR_LEN(key), &tmp2);
        } else {
            add_index_zval(return_value, index, &tmp2);
        }
    } ZEND_HASH_FOREACH_END();
}

static void server_request_upload_from_spec(zval *return_value, zval *file)
{
    zval *tmp = zend_hash_str_find(Z_ARRVAL_P(file), ZEND_STRL("tmp_name"));
    if( tmp && Z_TYPE_P(tmp) == IS_ARRAY ) {
        server_request_upload_from_nested(return_value, file, tmp);
    } else {
        ZVAL_ZVAL(return_value, file, 0, 0);
    }
}

void server_request_normalize_files(zval *return_value, zval *files)
{
    zend_string *key;
    zend_ulong index;
    zval *val;
    zval tmp = {0};

    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(files), index, key, val) {
        if( Z_TYPE_P(val) == IS_ARRAY ) {
            server_request_upload_from_spec(&tmp, val);
            if( key ) {
                add_assoc_zval_ex(return_value, ZSTR_VAL(key), ZSTR_LEN(key), &tmp);
            } else {
                add_index_zval(return_value, index, &tmp);
            }
        }
    } ZEND_HASH_FOREACH_END();
}
/* }}} */

/* {{{ server_request_obj_create */
static zend_object *server_request_obj_create(zend_class_entry *ce)
{
    zend_object *obj;

    obj = ecalloc(1, sizeof(*obj) + zend_object_properties_size(ce));
    zend_object_std_init(obj, ce);
    object_properties_init(obj, ce);
    obj->handlers = &ServerRequest_obj_handlers;

    return obj;
}
/* }}} */

/* {{{ server_request_clone_obj */
static zend_object *server_request_clone_obj(zval *zobject)
{
    zend_object * new_obj = std_object_handlers.clone_obj(zobject);
    new_obj->handlers = &ServerRequest_obj_handlers;
    return new_obj;
}
/* }}} */

/* {{{ server_request_assert_immutable */
static int server_request_is_immutable(zval *value)
{
    zval *val;
    switch( Z_TYPE_P(value) ) {
        case IS_NULL:
        case IS_TRUE:
        case IS_FALSE:
        case IS_LONG:
        case IS_DOUBLE:
        case IS_STRING:
            return 1;
        case IS_ARRAY:
            ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(value), val) {
                if( !server_request_is_immutable(val) ) {
                    return 0;
                }
            } ZEND_HASH_FOREACH_END();
            return 1;
        default:
            return 0;
    }
}
static void server_request_assert_immutable(zval *value, const char *desc, size_t desc_len)
{
    if( !server_request_is_immutable(value) ) {
        zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "All $%.*s values must be null, scalar, or array.", desc_len, desc);
    }
}
/* }}} */

/* {{{ server_request_object_default_has_property */
static int server_request_object_default_has_property(zval *object, zval *member, int has_set_exists, void **cache_slot)
{
    return 1;
}
/* }}} */

/* {{{ server_request_throw_readonly_exception */
static inline void server_request_throw_readonly_exception(zval *object, zval *member)
{
    zend_string *ce_name = Z_OBJCE_P(object)->name;
    zend_string *member_str = zval_get_string(member);
    zend_throw_exception_ex(spl_ce_RuntimeException, 0, "%.*s::$%.*s is read-only.", ZSTR_LEN(ce_name), ZSTR_VAL(ce_name), ZSTR_LEN(member_str), ZSTR_VAL(member_str));
    zend_string_release(member_str);
}
/* }}} */

/* {{{ server_request_object_default_read_property */
static zval *server_request_object_default_read_property(zval *object, zval *member, int type, void **cache_slot, zval *rv)
{
    zval *retval;
    ZVAL_UNDEF(rv);
    retval = std_object_handlers.read_property(object, member, type, cache_slot, rv);
    // Make sure the property can't be modified
    if( !Z_ISREF_P(rv) && (type == BP_VAR_W || type == BP_VAR_RW  || type == BP_VAR_UNSET) ) {
        SEPARATE_ZVAL(rv);
        server_request_throw_readonly_exception(object, member);
    }
    return retval;
}
/* }}} */

/* {{{ server_request_object_default_write_property */
static void server_request_object_default_write_property(zval *object, zval *member, zval *value, void **cache_slot)
{
    if( get_scope() != ServerRequest_ce_ptr ) {
        server_request_throw_readonly_exception(object, member);
    } else {
        std_object_handlers.write_property(object, member, value, cache_slot);
    }
}
/* }}} */

/* {{{ server_request_object_default_unset_property */
static void server_request_object_default_unset_property(zval *object, zval *member, void **cache_slot)
{
    if( get_scope() != ServerRequest_ce_ptr ) {
        server_request_throw_readonly_exception(object, member);
    } else {
        std_object_handlers.unset_property(object, member, cache_slot);
    }
}
/* }}} */

/* {{{ server_request_object_content_read_property */
static zval *server_request_object_content_read_property(zval *object, zval *member, int type, void **cache_slot, zval *rv)
{
    php_stream *stream;
    zend_string *str;

    if( (type == BP_VAR_W || type == BP_VAR_RW  || type == BP_VAR_UNSET) ) {
        server_request_throw_readonly_exception(object, member);
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

/* {{{ server_request_object_has_property */
static int server_request_object_has_property(zval *object, zval *member, int has_set_exists, void **cache_slot)
{
//    if( !Z_OBJCE_P(object)->__isset && !std_object_handlers.has_property(object, member, has_set_exists, cache_slot) ) {
//        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "ServerRequest::$%.*s does not exist.", Z_STRLEN_P(member), Z_STRVAL_P(member));
//        return 0;
//    }
    struct prop_handlers *hnd = zend_hash_str_find_ptr(&ServerRequest_prop_handlers, Z_STRVAL_P(member), Z_STRLEN_P(member));
    return (hnd ? hnd->has_property : std_object_handlers.has_property)(object, member, has_set_exists, cache_slot);
}
/* }}} */

/* {{{ server_request_object_read_property */
static zval *server_request_object_read_property(zval *object, zval *member, int type, void **cache_slot, zval *rv)
{
    if( !Z_OBJCE_P(object)->__get && !std_object_handlers.has_property(object, member, 2, cache_slot) ) {
        zend_string *ce_name = Z_OBJCE_P(object)->name;
        zend_string *member_str = zval_get_string(member);
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "%.*s::$%.*s does not exist.", ZSTR_LEN(ce_name), ZSTR_VAL(ce_name), ZSTR_LEN(member_str), ZSTR_VAL(member_str));
        zend_string_release(member_str);
        return rv;
    }
    struct prop_handlers *hnd = zend_hash_str_find_ptr(&ServerRequest_prop_handlers, Z_STRVAL_P(member), Z_STRLEN_P(member));
    return (hnd ? hnd->read_property : std_object_handlers.read_property)(object, member, type, cache_slot, rv);
}
/* }}} */

/* {{{ server_request_object_write_property */
static void server_request_object_write_property(zval *object, zval *member, zval *value, void **cache_slot)
{
    if( !Z_OBJCE_P(object)->__set && !std_object_handlers.has_property(object, member, 2, cache_slot) ) {
        server_request_throw_readonly_exception(object, member);
        return;
    }
    struct prop_handlers *hnd = zend_hash_str_find_ptr(&ServerRequest_prop_handlers, Z_STRVAL_P(member), Z_STRLEN_P(member));
    return (hnd ? hnd->write_property : std_object_handlers.write_property)(object, member, value, cache_slot);
}
/* }}} */

/* {{{ server_request_object_unset_property */
static void server_request_object_unset_property(zval *object, zval *member, void **cache_slot)
{
    struct prop_handlers *hnd = zend_hash_str_find_ptr(&ServerRequest_prop_handlers, Z_STRVAL_P(member), Z_STRLEN_P(member));
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
    zend_hash_str_update_mem(&ServerRequest_prop_handlers, name, name_length, &hnd, sizeof(hnd));
}
static inline void register_default_prop_handlers(const char *name, size_t name_length)
{
    register_prop_handlers(
        name,
        name_length,
        server_request_object_default_has_property,
        server_request_object_default_read_property,
        server_request_object_default_write_property,
        server_request_object_default_unset_property
    );
}
/* }}} */

/* {{{ proto ServerRequest::__construct([ array $globals ]) */
static inline void server_request_copy_global(
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
        // Assert immutable
        server_request_assert_immutable(tmp, glob_key, glob_key_length);
        if( EG(exception) ) {
            return;
        }
        // Update property value
        zend_update_property(ServerRequest_ce_ptr, obj, obj_key, obj_key_length, tmp);
        Z_TRY_ADDREF_P(tmp);
    }
}
#define server_request_copy_global_lit(obj, obj_key, glob, glob_key) server_request_copy_global(obj, ZEND_STRL(obj_key), glob, ZEND_STRL(glob_key))

static inline void server_request_set_forwarded(zval *object, zval *server)
{
    zval forwardedFor;
    zval forwardedHost;
    zval forwardedProto;
    zval forwarded;
    zval *tmp;
    zend_string *tmp_str;

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_X_FORWARDED_FOR"))) ) {
        array_init(&forwardedFor);
        server_request_parse_x_forwarded_for(&forwardedFor, Z_STRVAL_P(tmp), Z_STRLEN_P(tmp));
        zend_update_property(ServerRequest_ce_ptr, object, ZEND_STRL("forwardedFor"), &forwardedFor);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_X_FORWARDED_HOST"))) ) {
        convert_to_string(tmp);
        tmp_str = php_trim(Z_STR_P(tmp), ZEND_STRL(" \t\r\n\v"), 3);
        ZVAL_STR(&forwardedHost, tmp_str);
        zend_update_property(ServerRequest_ce_ptr, object, ZEND_STRL("forwardedHost"), &forwardedHost);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_X_FORWARDED_PROTO"))) ) {
        convert_to_string(tmp);
        tmp_str = php_trim(Z_STR_P(tmp), ZEND_STRL(" \t\r\n\v"), 3);
        ZVAL_STR(&forwardedProto, tmp_str);
        zend_update_property(ServerRequest_ce_ptr, object, ZEND_STRL("forwardedProto"), &forwardedProto);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_FORWARDED"))) ) {
        array_init(&forwarded);
        server_request_parse_forwarded(&forwarded, Z_STRVAL_P(tmp), Z_STRLEN_P(tmp));
        zend_update_property(ServerRequest_ce_ptr, object, ZEND_STRL("forwarded"), &forwarded);
    }
}

static inline void server_request_set_url(zval *object, zval *server)
{
    zend_string *tmp;
    php_url *url;
    zval arr = {0};

    tmp = server_request_detect_url(server);
    if( !tmp ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "Could not determine host for ServerRequest.");
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

    zend_update_property(ServerRequest_ce_ptr, object, ZEND_STRL("url"), &arr);

    php_url_free(url);
}

static inline void server_request_set_accept_by_name(zval *object, zval *server, const char *src, size_t src_length, const char *dest, size_t dest_length)
{
    zval val;
    zval *tmp;

    array_init(&val);
    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), src, src_length)) ) {
        server_request_parse_accept(&val, Z_STRVAL_P(tmp), Z_STRLEN_P(tmp));
    }
    zend_update_property(ServerRequest_ce_ptr, object, dest, dest_length, &val);
}

static inline void server_request_parse_accept_language(zval *lang)
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

static inline void server_request_set_accept_language(zval *object, zval *server)
{
    zval val;
    zval *tmp;

    array_init(&val);
    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_ACCEPT_LANGUAGE"))) ) {
        server_request_parse_accept(&val, Z_STRVAL_P(tmp), Z_STRLEN_P(tmp));
        server_request_parse_accept_language(&val);
    }
    zend_update_property(ServerRequest_ce_ptr, object, ZEND_STRL("acceptLanguage"), &val);
}

static inline void server_request_set_auth(zval *object, zval *server)
{
    zval *tmp;
    zval digest = {0};

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("PHP_AUTH_PW"))) ) {
        zend_update_property(ServerRequest_ce_ptr, object, ZEND_STRL("authPw"), tmp);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("PHP_AUTH_TYPE"))) ) {
        zend_update_property(ServerRequest_ce_ptr, object, ZEND_STRL("authType"), tmp);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("PHP_AUTH_USER"))) ) {
        zend_update_property(ServerRequest_ce_ptr, object, ZEND_STRL("authUser"), tmp);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("PHP_AUTH_DIGEST"))) ) {
        zend_string *str = zval_get_string(tmp);
        server_request_parse_digest_auth(&digest, ZSTR_VAL(str), ZSTR_LEN(str));
        zend_update_property(ServerRequest_ce_ptr, object, ZEND_STRL("authDigest"), &digest);
    }

}

static inline void server_request_set_content(zval *object, zval *server)
{
    zval *tmp;
    zval zv = {0};
    zval contentType = {0};

    // content body read moved to prop handler

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_CONTENT_MD5"))) ) {
        zend_update_property(ServerRequest_ce_ptr, object, ZEND_STRL("contentMd5"), tmp);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_CONTENT_LENGTH"))) ) {
        zend_update_property(ServerRequest_ce_ptr, object, ZEND_STRL("contentLength"), tmp);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_CONTENT_TYPE"))) && Z_TYPE_P(tmp) == IS_STRING ) {
        server_request_parse_content_type(&contentType, Z_STRVAL_P(tmp), Z_STRLEN_P(tmp));
        if( Z_TYPE(contentType) == IS_ARRAY ) {
            // contentType
            tmp = zend_hash_str_find(Z_ARRVAL(contentType), ZEND_STRL("value"));
            if( tmp ) {
                zend_update_property(ServerRequest_ce_ptr, object, ZEND_STRL("contentType"), tmp);
            }
            // charset
            tmp = zend_hash_str_find(Z_ARRVAL(contentType), ZEND_STRL("charset"));
            if( tmp ) {
                zend_update_property(ServerRequest_ce_ptr, object, ZEND_STRL("contentCharset"), tmp);
            }
        }
    }
}

PHP_METHOD(ServerRequest, __construct)
{
    zval *_this_zval;
    zval *init;
    zval *zv_globals = NULL;
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

    // Check and update _initialized property
    init = zend_read_property(ServerRequest_ce_ptr, _this_zval, ZEND_STRL("_initialized"), 0, &rv);
    if( zend_is_true(init) ) {
        zend_string *ce_name = Z_OBJCE_P(_this_zval)->name;
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "%.*s::__construct() called after construction.", ZSTR_LEN(ce_name), ZSTR_VAL(ce_name));
        return;
    }
    zend_update_property_bool(ServerRequest_ce_ptr, _this_zval, ZEND_STRL("_initialized"), 1);

    // Copy superglobals
    server_request_copy_global_lit(_this_zval, "env", zv_globals, "_ENV");
    server_request_copy_global_lit(_this_zval, "server", zv_globals, "_SERVER");
    server_request_copy_global_lit(_this_zval, "cookie", zv_globals, "_COOKIE");
    server_request_copy_global_lit(_this_zval, "files", zv_globals, "_FILES");
    server_request_copy_global_lit(_this_zval, "get", zv_globals, "_GET");
    server_request_copy_global_lit(_this_zval, "post", zv_globals, "_POST");

    // Check if previous step threw
    if( EG(exception) ) {
        return;
    }

    // Read back server property
    server = zend_read_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("server"), 0, &rv);

    // Internal setters that require server
    if( server && Z_TYPE_P(server) == IS_ARRAY ) {
        // method
        ZVAL_STRING(&method, "");
        zend_bool xhr = server_request_detect_method(&method, server);
        zend_update_property(ServerRequest_ce_ptr, _this_zval, ZEND_STRL("method"), &method);
        zend_update_property_bool(ServerRequest_ce_ptr, _this_zval, ZEND_STRL("xhr"), xhr);

        // headers
        server_request_normalize_headers(&headers, server);
        zend_update_property(ServerRequest_ce_ptr, _this_zval, ZEND_STRL("headers"), &headers);

        // forwarded
        server_request_set_forwarded(_this_zval, server);

        // url
        server_request_set_url(_this_zval, server);

        // accepts
        server_request_set_accept_by_name(_this_zval, server, ZEND_STRL("HTTP_ACCEPT"), ZEND_STRL("accept"));
        server_request_set_accept_by_name(_this_zval, server, ZEND_STRL("HTTP_ACCEPT_CHARSET"), ZEND_STRL("acceptCharset"));
        server_request_set_accept_by_name(_this_zval, server, ZEND_STRL("HTTP_ACCEPT_ENCODING"), ZEND_STRL("acceptEncoding"));
        server_request_set_accept_language(_this_zval, server);

        // auth
        server_request_set_auth(_this_zval, server);
        server_request_set_content(_this_zval, server);
    }

    // Read back files property
    files = zend_read_property(ServerRequest_ce_ptr, _this_zval, ZEND_STRL("files"), 0, &rv);

    if( files && Z_TYPE_P(files) == IS_ARRAY ) {
        array_init(&uploads);
        server_request_normalize_files(&uploads, files);
        zend_update_property(ServerRequest_ce_ptr, _this_zval, ZEND_STRL("uploads"), &uploads);
    }
}
/* }}} ServerRequest::__construct */

/* {{{ proto ServerRequest ServerRequest::withInput($input) */
PHP_METHOD(ServerRequest, withInput)
{
    zval *input;
    zval *_this_zval = getThis();
    zend_object *clone_obj;
    zval clone = {0};

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(input)
    ZEND_PARSE_PARAMETERS_END();

    server_request_assert_immutable(input, ZEND_STRL("input"));
    if( EG(exception) ) {
        return;
    }

    ZVAL_OBJ(&clone, Z_OBJ_HT_P(_this_zval)->clone_obj(_this_zval));
    if( EG(exception) ) {
        zval_dtor(&clone);
        return;
    }

    // Set property
    zend_update_property(ServerRequest_ce_ptr, &clone, ZEND_STRL("input"), input);

    if( EG(exception) ) {
        zval_dtor(&clone);
        return;
    }

    RETURN_ZVAL(&clone, 1, 1);
}
/* }}} ServerRequest::withInput */

/* {{{ proto ServerRequest ServerRequest::withParam(string $key, $val) */
PHP_METHOD(ServerRequest, withParam)
{
    zend_string *key;
    zval *val;
    zval *_this_zval = getThis();
    zend_object *clone_obj;
    zval clone = {0};
    zval member = {0};
    zval params = {0};
    zval *params_ptr;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(key)
        Z_PARAM_ZVAL(val)
    ZEND_PARSE_PARAMETERS_END();

    server_request_assert_immutable(val, ZEND_STRL("params"));
    if( EG(exception) ) {
        return;
    }

    ZVAL_OBJ(&clone, Z_OBJ_HT_P(_this_zval)->clone_obj(_this_zval));
    if( EG(exception) ) {
        zval_dtor(&clone);
        return;
    }

    // Set property
    ZVAL_STRING(&member, "params");
    params_ptr = std_object_handlers.get_property_ptr_ptr(&clone, &member, BP_VAR_RW, NULL);
    zval_dtor(&member);

    if( EG(exception) ) {
        zval_dtor(&clone);
        return;
    }

    if( params_ptr && Z_TYPE_P(params_ptr) == IS_ARRAY ) {
        add_assoc_zval_ex(params_ptr, ZSTR_VAL(key), ZSTR_LEN(key), val);
        Z_TRY_ADDREF_P(val);
    } else {
        array_init(&params);
        add_assoc_zval_ex(&params, ZSTR_VAL(key), ZSTR_LEN(key), val);
        zend_update_property(ServerRequest_ce_ptr, &clone, ZEND_STRL("params"), &params);
    }

    if( EG(exception) ) {
        zval_dtor(&clone);
        return;
    }

    RETURN_ZVAL(&clone, 1, 1);
}
/* }}} ServerRequest::withParam */

/* {{{ proto ServerRequest ServerRequest::withParams(array $params) */
PHP_METHOD(ServerRequest, withParams)
{
    zval *params;
    zval *_this_zval = getThis();
    zend_object *clone_obj;
    zval clone = {0};

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(params)
    ZEND_PARSE_PARAMETERS_END();

    server_request_assert_immutable(params, ZEND_STRL("params"));
    if( EG(exception) ) {
        return;
    }

    // Clone
    ZVAL_OBJ(&clone, Z_OBJ_HT_P(_this_zval)->clone_obj(_this_zval));
    if( EG(exception) ) {
        zval_dtor(&clone);
        return;
    }

    // Set property
    zend_update_property(ServerRequest_ce_ptr, &clone, ZEND_STRL("params"), params);

    if( EG(exception) ) {
        zval_dtor(&clone);
        return;
    }

    RETURN_ZVAL(&clone, 1, 1);
}
/* }}} ServerRequest::withParam */

/* {{{ proto ServerRequest ServerRequest::withoutParam(string $key) */
PHP_METHOD(ServerRequest, withoutParam)
{
    zend_string *key;
    zval *_this_zval = getThis();
    zend_object *clone_obj;
    zval clone = {0};
    zval member = {0};
    zval params = {0};
    zval *params_ptr;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_STR(key)
    ZEND_PARSE_PARAMETERS_END();

    // Clone
    ZVAL_OBJ(&clone, Z_OBJ_HT_P(_this_zval)->clone_obj(_this_zval));
    if( EG(exception) ) {
        zval_dtor(&clone);
        return;
    }

    // Set property
    ZVAL_STRING(&member, "params");
    params_ptr = std_object_handlers.get_property_ptr_ptr(&clone, &member, BP_VAR_RW, NULL);
    zval_dtor(&member);

    if( EG(exception) ) {
        zval_dtor(&clone);
        return;
    }

    if( params_ptr && Z_TYPE_P(params_ptr) == IS_ARRAY ) {
        ZVAL_ZVAL(&params, params_ptr, 1, 0);
        SEPARATE_ZVAL(&params);
        zend_hash_del(Z_ARRVAL(params), key);
        zend_update_property(ServerRequest_ce_ptr, &clone, ZEND_STRL("params"), &params);
    }

    if( EG(exception) ) {
        zval_dtor(&clone);
        return;
    }

    RETURN_ZVAL(&clone, 1, 1);
}
/* }}} ServerRequest::withoutParam */

/* {{{ proto ServerRequest ServerRequest::withoutParams([ array $keys ]) */
PHP_METHOD(ServerRequest, withoutParams)
{
    zval *keys = NULL;
    zval *_this_zval = getThis();
    zend_object *clone_obj;
    zval clone = {0};
    zval member = {0};
    zval params = {0};
    zval *params_ptr;
    zval *key = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY(keys)
    ZEND_PARSE_PARAMETERS_END();

    // Clone
    ZVAL_OBJ(&clone, Z_OBJ_HT_P(_this_zval)->clone_obj(_this_zval));
    if( EG(exception) ) {
        zval_dtor(&clone);
        return;
    }

    // Set property
    ZVAL_STRING(&member, "params");
    params_ptr = std_object_handlers.get_property_ptr_ptr(&clone, &member, BP_VAR_RW, NULL);
    zval_dtor(&member);

    if( EG(exception) ) {
        zval_dtor(&clone);
        return;
    }

    if( !keys || Z_TYPE_P(keys) != IS_ARRAY || !params_ptr || Z_TYPE_P(params_ptr) != IS_ARRAY ) {
        // clear
        array_init(&params);
    } else {
        ZVAL_ZVAL(&params, params_ptr, 1, 0);
        SEPARATE_ZVAL(&params);
        ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(keys), key) {
            if( Z_TYPE_P(key) == IS_STRING ) {
                if( zend_hash_exists(Z_ARRVAL(params), Z_STR_P(key))) {
                    zend_hash_del(Z_ARRVAL(params), Z_STR_P(key));
                }
            } else if( Z_TYPE_P(key) == IS_LONG ) {
                zend_hash_index_del(Z_ARRVAL(params), Z_LVAL_P(key));
            }
        } ZEND_HASH_FOREACH_END();
    }

    zend_update_property(ServerRequest_ce_ptr, &clone, ZEND_STRL("params"), &params);

    if( EG(exception) ) {
        zval_dtor(&clone);
        return;
    }

    RETURN_ZVAL(&clone, 1, 1);
}
/* }}} ServerRequest::withoutParams */

/* {{{ proto ServerRequest ServerRequest::withUrl(array $url) */
static inline void copy_url_key(const char * key, size_t key_length, zval *dest, zval *src)
{
    zval *tmp = zend_hash_str_find(Z_ARRVAL_P(src), key, key_length);
    if( tmp ) {
        add_assoc_zval_ex(dest, key, key_length, tmp);
    } else {
        add_assoc_null_ex(dest, key, key_length);
    }
}

PHP_METHOD(ServerRequest, withUrl)
{
    zval *url;
    zval *_this_zval = getThis();
    zend_object *clone_obj;
    zval clone = {0};
    zval zvurl = {0};

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(url)
    ZEND_PARSE_PARAMETERS_END();

    server_request_assert_immutable(url, ZEND_STRL("url"));
    if( EG(exception) ) {
        return;
    }

    // Clone
    ZVAL_OBJ(&clone, Z_OBJ_HT_P(_this_zval)->clone_obj(_this_zval));
    if( EG(exception) ) {
        zval_dtor(&clone);
        return;
    }

    // Set property
    array_init(&zvurl);
    copy_url_key(ZEND_STRL("scheme"), &zvurl, url);
    copy_url_key(ZEND_STRL("host"), &zvurl, url);
    copy_url_key(ZEND_STRL("port"), &zvurl, url);
    copy_url_key(ZEND_STRL("user"), &zvurl, url);
    copy_url_key(ZEND_STRL("pass"), &zvurl, url);
    copy_url_key(ZEND_STRL("path"), &zvurl, url);
    copy_url_key(ZEND_STRL("query"), &zvurl, url);
    copy_url_key(ZEND_STRL("fragment"), &zvurl, url);

    zend_update_property(ServerRequest_ce_ptr, &clone, ZEND_STRL("url"), &zvurl);

    if( EG(exception) ) {
        zval_dtor(&clone);
        return;
    }

    RETURN_ZVAL(&clone, 1, 1);
}
/* }}} ServerRequest::withUrl */

/* {{{ proto array ServerRequest::parseAccept(string $header) */
PHP_METHOD(ServerRequest, parseAccept)
{
    zend_string *header;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(header)
    ZEND_PARSE_PARAMETERS_END();

    //array_init(return_value);
    server_request_parse_accept(return_value, ZSTR_VAL(header), ZSTR_LEN(header));

}
/* }}} ServerRequest::parseAccept */

/* {{{ proto array ServerRequest::parseContentType(string $header) */
PHP_METHOD(ServerRequest, parseContentType)
{
    zend_string *header;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(header)
    ZEND_PARSE_PARAMETERS_END();

    server_request_parse_content_type(return_value, ZSTR_VAL(header), ZSTR_LEN(header));
}
/* }}} ServerRequest::parseContentType */

/* {{{ proto array ServerRequest::parseDigestAuth(string $header) */
PHP_METHOD(ServerRequest, parseDigestAuth)
{
    zend_string *header;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(header)
    ZEND_PARSE_PARAMETERS_END();

    server_request_parse_digest_auth(return_value, ZSTR_VAL(header), ZSTR_LEN(header));
}
/* }}} ServerRequest::parseDigestAuth */

/* {{{ ServerRequest methods */
static zend_function_entry ServerRequest_methods[] = {
    PHP_ME(ServerRequest, __construct, ServerRequest_construct_args, ZEND_ACC_PUBLIC)
    PHP_ME(ServerRequest, withInput, ServerRequest_withInput_args, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(ServerRequest, withParam, ServerRequest_withParam_args, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(ServerRequest, withParams, ServerRequest_withParams_args, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(ServerRequest, withoutParam, ServerRequest_withoutParam_args, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(ServerRequest, withoutParams, ServerRequest_withoutParams_args, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(ServerRequest, withUrl, ServerRequest_withUrl_args, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(ServerRequest, parseAccept, ServerRequest_parseAccept_args, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ServerRequest, parseContentType, ServerRequest_parseContentType_args, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ServerRequest, parseDigestAuth, ServerRequest_parseDigestAuth_args, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_FE_END
};
/* }}} ServerRequest methods */

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(serverrequest)
{
    zend_class_entry ce;

    zend_hash_init(&ServerRequest_prop_handlers, 0, NULL, NULL, 1);

    memcpy(&ServerRequest_obj_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    ServerRequest_obj_handlers.has_property = server_request_object_has_property;
    ServerRequest_obj_handlers.read_property = server_request_object_read_property;
    ServerRequest_obj_handlers.write_property = server_request_object_write_property;
    ServerRequest_obj_handlers.unset_property = server_request_object_unset_property;
    ServerRequest_obj_handlers.get_property_ptr_ptr = NULL;
    ServerRequest_obj_handlers.clone_obj = server_request_clone_obj;

    INIT_CLASS_ENTRY(ce, "ServerRequest", ServerRequest_methods);
    ServerRequest_ce_ptr = zend_register_internal_class(&ce);
    ServerRequest_ce_ptr->create_object = server_request_obj_create;

    zend_declare_property_bool(ServerRequest_ce_ptr, ZEND_STRL("_initialized"), 0, ZEND_ACC_PRIVATE);

    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("accept"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("accept"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("acceptCharset"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("acceptCharset"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("acceptEncoding"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("acceptEncoding"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("acceptLanguage"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("acceptLanguage"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("authDigest"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("authDigest"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("authPw"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("authPw"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("authType"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("authType"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("authUser"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("authUser"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("content"), ZEND_ACC_PUBLIC);
    register_prop_handlers(
        ZEND_STRL("content"),
        server_request_object_default_has_property,
        server_request_object_content_read_property,
        server_request_object_default_write_property,
        server_request_object_default_unset_property
    );
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("contentCharset"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("contentCharset"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("contentLength"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("contentLength"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("contentMd5"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("contentMd5"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("contentType"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("contentType"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("cookie"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("cookie"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("env"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("env"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("files"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("files"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("forwarded"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("forwarded"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("forwardedFor"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("forwardedFor"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("forwardedHost"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("forwardedHost"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("forwardedProto"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("forwardedProto"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("get"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("get"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("headers"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("headers"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("input"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("input"));
    zend_declare_property_string(ServerRequest_ce_ptr, ZEND_STRL("method"), "", ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("method"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("params"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("params"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("post"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("post"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("server"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("server"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("uploads"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("uploads"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("url"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("url"));
    zend_declare_property_bool(ServerRequest_ce_ptr, ZEND_STRL("xhr"), 0, ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("xhr"));

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION */
PHP_MSHUTDOWN_FUNCTION(serverrequest)
{
    zend_hash_destroy(&ServerRequest_prop_handlers);

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
