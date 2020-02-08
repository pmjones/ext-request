#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>
#include <string.h>

#include "main/php.h"
#include "main/php_ini.h"
#include "main/php_output.h"
#include "main/php_streams.h"
#include "main/SAPI.h"
#include "main/snprintf.h"

#include "ext/standard/info.h"
#include "ext/standard/head.h"
#include "ext/standard/php_string.h"
#include "ext/standard/url.h"
#include "ext/spl/spl_exceptions.h"

#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_interfaces.h"
#include "Zend/zend_portability.h"
#include "Zend/zend_types.h"
#include "Zend/zend_smart_str.h"

#include "php_request.h"

/* {{{ PHP_MINIT_FUNCTION */
static PHP_MINIT_FUNCTION(request)
{
    PHP_MINIT(serverrequest)(INIT_FUNC_ARGS_PASSTHRU);
    PHP_MINIT(serverresponse)(INIT_FUNC_ARGS_PASSTHRU);
    PHP_MINIT(serverresponsesender)(INIT_FUNC_ARGS_PASSTHRU);
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
static PHP_MINFO_FUNCTION(request)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "Version", PHP_REQUEST_VERSION);
    php_info_print_table_end();
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION */
static PHP_MSHUTDOWN_FUNCTION(request)
{
    PHP_MSHUTDOWN(serverrequest)(SHUTDOWN_FUNC_ARGS_PASSTHRU);
    return SUCCESS;
}
/* }}} */

/* {{{ request_deps */
static const zend_module_dep request_deps[] = {
    ZEND_MOD_REQUIRED("spl")
    ZEND_MOD_END
};
/* }}} */

zend_module_entry request_module_entry = {
    STANDARD_MODULE_HEADER_EX, NULL,
    request_deps,                       /* Deps */
    PHP_REQUEST_NAME,                   /* Name */
    NULL,                               /* Functions */
    PHP_MINIT(request),                 /* MINIT */
    PHP_MSHUTDOWN(request),             /* MSHUTDOWN */
    NULL,                               /* RINIT */
    NULL,                               /* RSHUTDOWN */
    PHP_MINFO(request),                 /* MINFO */
    PHP_REQUEST_VERSION,                /* Version */
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_REQUEST
ZEND_GET_MODULE(request)      // Common for all PHP extensions which are build as shared modules
#endif

zend_class_entry *ServerRequest_ce_ptr;
zend_class_entry *ServerResponse_ce_ptr;
zend_class_entry *ServerResponseSender_ce_ptr;

/* {{{ server_request_normalize_header_name */
void server_request_normalize_header_name(char *key, size_t key_length)
{
    register char *r = key;
    register char *r_end = r + key_length - 1;

    *r = tolower((unsigned char) *r);
    r++;
    for( ; r <= r_end; r++ ) {
        if( *r == '_' ) {
            *r = '-';
        } else {
            *r = tolower((unsigned char) *r);
        }
    }
}

static zend_string *server_request_normalize_header_name_ex(zend_string *in)
{
    zend_string *out = php_trim(in, ZEND_STRL(" \t\r\n\v"), 3);
    server_request_normalize_header_name(ZSTR_VAL(out), ZSTR_LEN(out));
    zend_string_forget_hash_val(out);
    zend_string_hash_val(out);
    return out;
}
/* }}} server_request_normalize_header_name */

/* ServerRequest ************************************************************ */

extern void server_request_parse_accept(zval *return_value, const unsigned char *str, size_t len);
extern void server_request_parse_content_type(zval *return_value, const unsigned char *str, size_t len);
extern void server_request_parse_digest_auth(zval *return_value, const unsigned char *str, size_t len);
extern void server_request_parse_x_forwarded_for(zval *return_value, const unsigned char *str, size_t len);
extern void server_request_parse_x_forwarded(zval *return_value, const unsigned char *str, size_t len);
extern void server_request_normalize_header_name(char *key, size_t key_length);


static zend_object_handlers ServerRequest_obj_handlers;
static HashTable ServerRequest_prop_handlers;

struct prop_handlers {
    zend_object_has_property_t has_property;
    zend_object_read_property_t read_property;
    zend_object_write_property_t write_property;
    zend_object_unset_property_t unset_property;
};

/* {{{ Argument Info */
ZEND_BEGIN_ARG_INFO_EX(ServerRequest_construct_args, 0, 0, 1)
    ZEND_ARG_ARRAY_INFO(0, globals, 0)
    ZEND_ARG_TYPE_INFO(0, content, IS_STRING, 0)
ZEND_END_ARG_INFO()
/* }}} Argument Info */

/* {{ server_request_detect_method */
static void server_request_detect_method(zval *return_value, zval *server)
{
    zend_string *tmp;
    zval *val;
    zend_string *method;

    // determine method from request
    val = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("REQUEST_METHOD"));
    if( !val || Z_TYPE_P(val) != IS_STRING ) {
        ZVAL_NULL(return_value);
        return;
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
static inline const unsigned char *server_request_extract_port_from_host(const unsigned char *host, size_t len)
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

static inline zend_string *server_request_extract_host_from_server(zval *server)
{
    zval *tmp;
    zend_string *host;
    const char *fake_host = "___";

    // Get host
    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_HOST"))) &&
        Z_TYPE_P(tmp) == IS_STRING ) {
        host = Z_STR_P(tmp);
    } else if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("SERVER_NAME"))) &&
               Z_TYPE_P(tmp) == IS_STRING ) {
        host = Z_STR_P(tmp);
    } else {
        host = zend_string_init(fake_host, strlen(fake_host), 0);
    }

    return host;
}

static inline zend_long server_request_extract_port_from_server(zval *server, zend_string *host)
{
    zval *tmp;

    // Get port
    if( NULL != server_request_extract_port_from_host(ZSTR_VAL(host), ZSTR_LEN(host)) ) {
        // no need to extract
    } else if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("SERVER_PORT"))) ) {
        return zval_get_long(tmp);
    }

    return 0;
}

static inline zend_string *server_request_extract_uri_from_server(zval *server)
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
    const char *fake_host = "___";

    host = server_request_extract_host_from_server(server);
    port = server_request_extract_port_from_server(server, host);
    uri = server_request_extract_uri_from_server(server);

    if( ! strcmp(ZSTR_VAL(host), fake_host) && ! port && ! uri ) {
        return NULL;
    }

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
static void server_request_normalize_headers(zval *return_value, zval *server)
{
    zend_string *key;
    zend_ulong index;
    zval *val;
    zend_string *tmp;

    char http_str[] = "HTTP_";
    size_t http_len = sizeof(http_str) - 1;

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

static void server_request_normalize_files(zval *return_value, zval *files)
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
static int server_request_value_is_immutable(zval *value)
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
                if( !server_request_value_is_immutable(val) ) {
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
    if( !server_request_value_is_immutable(value) ) {
        zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "All $%.*s values must be null, scalar, or array.", (int)desc_len, desc);
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
    zend_throw_exception_ex(spl_ce_RuntimeException, 0, "%.*s::$%.*s is read-only.", (int)ZSTR_LEN(ce_name), ZSTR_VAL(ce_name), (int)ZSTR_LEN(member_str), ZSTR_VAL(member_str));
    zend_string_release(member_str);
}
/* }}} */

/* {{{ server_request_object_default_read_property */
static zval *server_request_object_default_read_property(zval *object, zval *member, int type, void **cache_slot, zval *rv)
{
    zval *retval;
    php_stream *stream;
    zend_string *str;

    ZVAL_UNDEF(rv);
    retval = std_object_handlers.read_property(object, member, type, cache_slot, rv);

    // Make sure the property can't be modified
    if( !Z_ISREF_P(rv) && (type == BP_VAR_W || type == BP_VAR_RW  || type == BP_VAR_UNSET) ) {
        SEPARATE_ZVAL(rv);
        server_request_throw_readonly_exception(object, member);
    }

    if (strcmp(Z_STRVAL_P(member), "content")) {
        // non-content member
        return retval;
    }

    if (Z_TYPE_P(retval) == IS_STRING) {
        // content is already a string
        return retval;
    }

    // read from php://input on the fly
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

/* {{{ server_request_object_default_write_property */
static void server_request_object_default_write_property(zval *object, zval *member, zval *value, void **cache_slot)
{
    if( zend_get_executed_scope() != ServerRequest_ce_ptr ) {
        server_request_throw_readonly_exception(object, member);
    } else {
        std_object_handlers.write_property(object, member, value, cache_slot);
    }
}
/* }}} */

/* {{{ server_request_object_default_unset_property */
static void server_request_object_default_unset_property(zval *object, zval *member, void **cache_slot)
{
    if( zend_get_executed_scope() != ServerRequest_ce_ptr ) {
        server_request_throw_readonly_exception(object, member);
    } else {
        std_object_handlers.unset_property(object, member, cache_slot);
    }
}
/* }}} */

/* {{{ server_request_object_has_property */
static int server_request_object_has_property(zval *object, zval *member, int has_set_exists, void **cache_slot)
{
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
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "%.*s::$%.*s does not exist.", (int)ZSTR_LEN(ce_name), ZSTR_VAL(ce_name), (int)ZSTR_LEN(member_str), ZSTR_VAL(member_str));
        zend_string_release(member_str);
        ZVAL_NULL(rv);
        return rv;
    }
    struct prop_handlers *hnd = zend_hash_str_find_ptr(&ServerRequest_prop_handlers, Z_STRVAL_P(member), Z_STRLEN_P(member));
    return (hnd ? hnd->read_property : std_object_handlers.read_property)(object, member, type, cache_slot, rv);
}
/* }}} */

/* {{{ server_request_object_write_property */
static void server_request_object_write_property(zval *object, zval *member, zval *value, void **cache_slot)
{
    if( !Z_OBJCE_P(object)->__get && !std_object_handlers.has_property(object, member, 2, cache_slot) ) {
        zend_string *ce_name = Z_OBJCE_P(object)->name;
        zend_string *member_str = zval_get_string(member);
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "%.*s::$%.*s does not exist.", (int)ZSTR_LEN(ce_name), ZSTR_VAL(ce_name), (int)ZSTR_LEN(member_str), ZSTR_VAL(member_str));
        zend_string_release(member_str);
        return;
    }

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

/* {{{ server_request_object_get_property_ptr_ptr */
static zval *server_request_object_get_property_ptr_ptr(zend_object *object, zend_string *name, int type, void **cache_slot)
{
    return NULL;
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
    hnd.has_property = has_property ? has_property : std_object_handlers.has_property;
    hnd.read_property = read_property ? read_property : std_object_handlers.read_property;
    hnd.write_property = write_property ? write_property : std_object_handlers.write_property;
    hnd.unset_property = unset_property ? unset_property : std_object_handlers.unset_property;
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
static inline void server_request_copy_global_prop(
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

static inline void server_request_init_array_prop(
    zval *obj,
    const char *obj_key,
    size_t obj_key_length
) {
    zval *tmp;
    array_init(&tmp);
    zend_update_property(ServerRequest_ce_ptr, obj, obj_key, obj_key_length, &tmp);
}

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
    const char *fake_host = "___";

    tmp = server_request_detect_url(server);
    if( !tmp ) {
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
        add_assoc_string(&arr, "scheme", ZSTR_VAL(url->scheme));
    } else {
        add_assoc_null(&arr, "scheme");
    }
    if( url->host && strcmp(ZSTR_VAL(url->host), fake_host)) {
        // not a fake
        add_assoc_string(&arr, "host", ZSTR_VAL(url->host));
    } else {
        add_assoc_null(&arr, "host");
    }
    if( url->port ) {
        add_assoc_long(&arr, "port", url->port);
    } else {
        add_assoc_null(&arr, "port");
    }
    if( url->user ) {
        add_assoc_string(&arr, "user", ZSTR_VAL(url->user));
    } else {
        add_assoc_null(&arr, "user");
    }
    if( url->pass ) {
        add_assoc_string(&arr, "pass", ZSTR_VAL(url->pass));
    } else {
        add_assoc_null(&arr, "pass");
    }
    if( url->path ) {
        add_assoc_string(&arr, "path", ZSTR_VAL(url->path));
    } else {
        add_assoc_null(&arr, "path");
    }
    if( url->query ) {
        add_assoc_string(&arr, "query", ZSTR_VAL(url->query));
    } else {
        add_assoc_null(&arr, "query");
    }
    if( url->fragment ) {
        add_assoc_string(&arr, "fragment", ZSTR_VAL(url->fragment));
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
    tmp = zend_hash_str_find(Z_ARRVAL_P(server), src, src_length);
    if( tmp && Z_TYPE_P(tmp) == IS_STRING ) {
        server_request_parse_accept(&val, Z_STRVAL_P(tmp), Z_STRLEN_P(tmp));
        zend_update_property(ServerRequest_ce_ptr, object, dest, dest_length, &val);
    }
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
        zend_update_property(ServerRequest_ce_ptr, object, ZEND_STRL("acceptLanguage"), &val);
    }
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

static inline void server_request_set_content_length(zval *object, zval *length)
{
    zend_string *tmp = php_trim(Z_STR_P(length), ZEND_STRL("0123456789 \t\r\n\v"), 3);
    if( zend_string_equals_literal(tmp, "") ) {
        zend_string_release(tmp);
        convert_to_long(length);
        zend_update_property(ServerRequest_ce_ptr, object, ZEND_STRL("contentLength"), length);
    }
}

static inline void server_request_set_content(zval *object, zval *server)
{
    zval *tmp;
    zval zv = {0};
    zval contentType = {0};

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_CONTENT_MD5"))) ) {
        zend_update_property(ServerRequest_ce_ptr, object, ZEND_STRL("contentMd5"), tmp);
    }

    if( tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_CONTENT_LENGTH")) ) {
        server_request_set_content_length(object, tmp);
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

/* {{{ ServerRequest::__construct */
PHP_METHOD(ServerRequest, __construct)
{
    zval *_this_zval;
    zval *init;
    zval *globals = NULL;
    zval *content = NULL;
    zval *server;
    zval *files;
    zval rv = {0};
    zval method = {0};
    zval headers = {0};
    zval uploads = {0};
    zval *xreqwith_val;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_ARRAY(globals)
        Z_PARAM_OPTIONAL
        Z_PARAM_ZVAL(content)
    ZEND_PARSE_PARAMETERS_END();

    _this_zval = getThis();

    // Check and update _initialized property
    init = zend_read_property(ServerRequest_ce_ptr, _this_zval, ZEND_STRL("_initialized"), 0, &rv);
    if( zend_is_true(init) ) {
        zend_string *ce_name = Z_OBJCE_P(_this_zval)->name;
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "%.*s::__construct() called after construction.", (int)ZSTR_LEN(ce_name), ZSTR_VAL(ce_name));
        return;
    }
    zend_update_property_bool(ServerRequest_ce_ptr, _this_zval, ZEND_STRL("_initialized"), 1);

    // initialize array properties
    server_request_init_array_prop(_this_zval, ZEND_STRL("accept"));
    server_request_init_array_prop(_this_zval, ZEND_STRL("acceptCharset"));
    server_request_init_array_prop(_this_zval, ZEND_STRL("acceptEncoding"));
    server_request_init_array_prop(_this_zval, ZEND_STRL("acceptLanguage"));
    server_request_init_array_prop(_this_zval, ZEND_STRL("authDigest"));
    server_request_init_array_prop(_this_zval, ZEND_STRL("cookie"));
    server_request_init_array_prop(_this_zval, ZEND_STRL("env"));
    server_request_init_array_prop(_this_zval, ZEND_STRL("files"));
    server_request_init_array_prop(_this_zval, ZEND_STRL("forwarded"));
    server_request_init_array_prop(_this_zval, ZEND_STRL("forwardedFor"));
    server_request_init_array_prop(_this_zval, ZEND_STRL("get"));
    server_request_init_array_prop(_this_zval, ZEND_STRL("headers"));
    server_request_init_array_prop(_this_zval, ZEND_STRL("post"));
    server_request_init_array_prop(_this_zval, ZEND_STRL("server"));
    server_request_init_array_prop(_this_zval, ZEND_STRL("uploads"));
    server_request_init_array_prop(_this_zval, ZEND_STRL("url"));

    // Copy superglobals
    server_request_copy_global_prop(_this_zval, ZEND_STRL("env"),    globals, ZEND_STRL("_ENV"));
    server_request_copy_global_prop(_this_zval, ZEND_STRL("server"), globals, ZEND_STRL("_SERVER"));
    server_request_copy_global_prop(_this_zval, ZEND_STRL("cookie"), globals, ZEND_STRL("_COOKIE"));
    server_request_copy_global_prop(_this_zval, ZEND_STRL("files"),  globals, ZEND_STRL("_FILES"));
    server_request_copy_global_prop(_this_zval, ZEND_STRL("get"),    globals, ZEND_STRL("_GET"));
    server_request_copy_global_prop(_this_zval, ZEND_STRL("post"),   globals, ZEND_STRL("_POST"));

    // Check if previous step threw
    if( EG(exception) ) {
        return;
    }

    // Read back server property
    server = zend_read_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("server"), 0, &rv);

    // Internal setters that require server
    if( server && Z_TYPE_P(server) == IS_ARRAY ) {
        // headers
        server_request_normalize_headers(&headers, server);
        zend_update_property(ServerRequest_ce_ptr, _this_zval, ZEND_STRL("headers"), &headers);

        // requestedWith
        xreqwith_val = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_X_REQUESTED_WITH"));
        if( xreqwith_val && Z_TYPE_P(xreqwith_val) == IS_STRING ) {
            zend_update_property(ServerRequest_ce_ptr, _this_zval, ZEND_STRL("requestedWith"), xreqwith_val);
        }

        // method
        server_request_detect_method(&method, server);
        zend_update_property(ServerRequest_ce_ptr, _this_zval, ZEND_STRL("method"), &method);

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

    if (content && Z_TYPE_P(content) == IS_STRING) {
        zend_update_property(ServerRequest_ce_ptr, _this_zval, ZEND_STRL("content"), content);
    }
}
/* }}} ServerRequest::__construct */

/* {{{ ServerRequest methods */
static zend_function_entry ServerRequest_methods[] = {
    PHP_ME(ServerRequest, __construct, ServerRequest_construct_args, ZEND_ACC_PUBLIC)
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
    ServerRequest_obj_handlers.get_property_ptr_ptr = server_request_object_get_property_ptr_ptr;
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
    register_default_prop_handlers(ZEND_STRL("content"));
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
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("method"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("method"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("post"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("post"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("requestedWith"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("requestedWith"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("server"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("server"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("uploads"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("uploads"));
    zend_declare_property_null(ServerRequest_ce_ptr, ZEND_STRL("url"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("url"));
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

static inline void smart_str_appendz_ex(smart_str *dest, zval *zv, zend_bool persistent)
{
    zend_string *tmp;
    if( Z_TYPE_P(zv) == IS_STRING ) {
        smart_str_append_ex(dest, Z_STR_P(zv), persistent);
    } else {
        tmp = zval_get_string(zv);
        smart_str_append_ex(dest, Z_STR_P(zv), persistent);
        zend_string_release(tmp);
    }
}

static inline void smart_str_appendz(smart_str *dest, zval *zv)
{
    return smart_str_appendz_ex(dest, zv, 0);
}

/* ServerResponse *********************************************************** */

/* {{{ ServerResponse Argument Info */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponse_getVersion_args, 0, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponse_setVersion_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, version, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponse_getCode_args, 0, 0, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponse_setCode_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, code, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponse_getHeaders_args, 0, 0, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponse_setHeader_args, 0, 2, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, label, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponse_addHeader_args, 0, 2, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, label, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponse_unsetHeader_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, label, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponse_unsetHeaders_args, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponse_getCookies_args, 0, 0, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponse_setCookie_args, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
    ZEND_ARG_INFO(0, expires_or_options)
    ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, domain, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, secure, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, httponly, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponse_unsetCookie_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponse_unsetCookies_args, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ServerResponse_getContent_args, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponse_setContent_args, 0, 1, IS_VOID, 1)
    ZEND_ARG_INFO(0, content)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponse_addHeaderCallback_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponse_setHeaderCallbacks_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, callbacks, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponse_getHeaderCallbacks_args, 0, 0, IS_ARRAY, 1)
ZEND_END_ARG_INFO()
/* }}} Argument Info */

/* {{{ proto string ServerResponse::getVersion() */
static zval *server_response_get_version(zval *response)
{
    return zend_read_property(ServerResponse_ce_ptr, response, ZEND_STRL("version"), 0, NULL);
}

PHP_METHOD(ServerResponse, getVersion)
{
    zval *_this_zval = getThis();

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    RETVAL_ZVAL(server_response_get_version(_this_zval), 1, 0);
}
/* }}} ServerResponse::getVersion */

/* {{{ proto void ServerResponse::setVersion(string $version) */
PHP_METHOD(ServerResponse, setVersion)
{
    zval *_this_zval = getThis();
    zend_string *version;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(version)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property_str(ServerResponse_ce_ptr, _this_zval, ZEND_STRL("version"), version);
}
/* }}} ServerResponse::setVersion */

/* {{{ proto int ServerResponse::getCode() */
static zval *server_response_get_code(zval *response)
{
    return zend_read_property(ServerResponse_ce_ptr, response, ZEND_STRL("code"), 0, NULL);
}

PHP_METHOD(ServerResponse, getCode)
{
    zval *_this_zval = getThis();

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    RETVAL_ZVAL(server_response_get_code(_this_zval), 1, 0);
}
/* }}} ServerResponse::getCode */

/* {{{ proto void ServerResponse::setCode(int $version) */
PHP_METHOD(ServerResponse, setCode)
{
    zval *_this_zval = getThis();
    zend_long code;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(code)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property_long(ServerResponse_ce_ptr, _this_zval, ZEND_STRL("code"), code);
}
/* }}} ServerResponse::setCode */

/* {{{ proto array ServerResponse::getHeaders() */
static zval *server_response_get_headers(zval *response)
{
    return zend_read_property(ServerResponse_ce_ptr, response, ZEND_STRL("headers"), 0, NULL);
}

PHP_METHOD(ServerResponse, getHeaders)
{
    zval *_this_zval = getThis();

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    RETVAL_ZVAL(server_response_get_headers(_this_zval), 1, 0);
}
/* }}} ServerResponse::getHeaders */

/* {{{ proto void ServerResponse::setHeader(string $label, string $value) */
static void server_response_set_header(zval *response, zend_string *label, zend_string *value, zend_bool replace)
{
    zval member;
    zval *prop_ptr;
    zend_string *normal_label;
    zend_string *value_str;
    zend_string *tmp;
    zval *prev_header = NULL;
    smart_str buf = {0};

    // Read property pointer
    if( !Z_OBJ_HT_P(response)->get_property_ptr_ptr ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "ServerResponse::setHeader requires get_property_ptr_ptr");
        return;
    }

    ZVAL_STRING(&member, "headers");
    prop_ptr = Z_OBJ_HT_P(response)->get_property_ptr_ptr(response, &member, BP_VAR_RW, NULL);
    if( !prop_ptr || Z_TYPE_P(prop_ptr) != IS_ARRAY ) {
        convert_to_array(prop_ptr);
    }

    // Normalize label
    normal_label = server_request_normalize_header_name_ex(label);

    if( !ZSTR_LEN(normal_label) ) {
        zend_string_release(normal_label);
        zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "Header label cannot be blank");
        return;
    }

    // append to previous value?
    if( !replace ) {
        prev_header = zend_hash_find(Z_ARRVAL_P(prop_ptr), normal_label);
        if( prev_header ) {
            smart_str_appendz(&buf, prev_header);
            smart_str_appendl_ex(&buf, ZEND_STRL(", "), 0);
        }
    }

    // trim whitespace
    value_str = php_trim(value, ZEND_STRL(" \t\r\n\v"), 3);

    if( !ZSTR_LEN(value_str) ) {
        smart_str_free(&buf);
        zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "Header value cannot be blank");
    } else {
        // Set/append value
        smart_str_append_ex(&buf, value_str, 0);
        smart_str_0(&buf);
        add_assoc_str_ex(prop_ptr, ZSTR_VAL(normal_label), ZSTR_LEN(normal_label), buf.s);
    }

    zend_string_release(value_str);
    zend_string_release(normal_label);
}

PHP_METHOD(ServerResponse, setHeader)
{
    zend_string *label;
    zend_string *value;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(label)
        Z_PARAM_STR(value)
    ZEND_PARSE_PARAMETERS_END();

    server_response_set_header(getThis(), label, value, 1);
}
/* }}} ServerResponse::setHeader */

/* {{{ proto void ServerResponse::addHeader(string $label, string $value) */
PHP_METHOD(ServerResponse, addHeader)
{
    zend_string *label;
    zend_string *value;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(label)
        Z_PARAM_STR(value)
    ZEND_PARSE_PARAMETERS_END();

    server_response_set_header(getThis(), label, value, 0);
}
/* }}} ServerResponse::addHeader */

/* {{{ proto void ServerResponse::unsetHeader(string $label) */
static void server_response_unset_header(zval *response, zend_string *label)
{
    zval member;
    zval *prop_ptr;
    zend_string *normal_label;

    // Read property pointer
    if( !Z_OBJ_HT_P(response)->get_property_ptr_ptr ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "ServerResponse::unsetHeader requires get_property_ptr_ptr");
        return;
    }

    ZVAL_STRING(&member, "headers");
    prop_ptr = Z_OBJ_HT_P(response)->get_property_ptr_ptr(response, &member, BP_VAR_RW, NULL);
    if( !prop_ptr || Z_TYPE_P(prop_ptr) != IS_ARRAY ) {
        return;
    }

    normal_label = server_request_normalize_header_name_ex(label);

    if( ZSTR_LEN(normal_label) ) {
        zend_hash_del(Z_ARRVAL_P(prop_ptr), normal_label);
    }

    zend_string_release(normal_label);
}

PHP_METHOD(ServerResponse, unsetHeader)
{
    zend_string *label;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(label)
    ZEND_PARSE_PARAMETERS_END();

    server_response_unset_header(getThis(), label);
}
/* }}} ServerResponse::unsetHeader */

/* {{{ proto void ServerResponse::unsetHeaders() */
PHP_METHOD(ServerResponse, unsetHeaders)
{
    zval *_this_zval = getThis();

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property_null(ServerResponse_ce_ptr, _this_zval, ZEND_STRL("headers"));
}
/* }}} ServerResponse::unsetHeaders */

/* {{{ proto array ServerResponse::getCookies() */
static zval *server_response_get_cookies(zval *response)
{
    return zend_read_property(ServerResponse_ce_ptr, response, ZEND_STRL("cookies"), 0, NULL);
}

PHP_METHOD(ServerResponse, getCookies)
{
    zval *_this_zval = getThis();

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    RETVAL_ZVAL(server_response_get_cookies(_this_zval), 1, 0);
}
/* }}} ServerResponse::getCookies */

/* {{{ proto void ServerResponse::setCookie(string name [, string value [, int expires [, string path [, string domain [, bool secure[, bool httponly]]]]]]) */
static void php_head_parse_cookie_options_array(zval *options, zend_long *expires, zend_string **path, zend_string **domain, zend_bool *secure, zend_bool *httponly, zend_string **samesite)
{
    /* copied from ext/standard/head.c, as it it not published in head.h */

    int found = 0;
    zend_string *key;
    zval *value;

    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(options), key, value) {
        if (key) {
            if (zend_string_equals_literal_ci(key, "expires")) {
                *expires = zval_get_long(value);
                found++;
            } else if (zend_string_equals_literal_ci(key, "path")) {
                *path = zval_get_string(value);
                found++;
            } else if (zend_string_equals_literal_ci(key, "domain")) {
                *domain = zval_get_string(value);
                found++;
            } else if (zend_string_equals_literal_ci(key, "secure")) {
                *secure = zval_is_true(value);
                found++;
            } else if (zend_string_equals_literal_ci(key, "httponly")) {
                *httponly = zval_is_true(value);
                found++;
            } else if (zend_string_equals_literal_ci(key, "samesite")) {
                *samesite = zval_get_string(value);
                found++;
            } else {
                php_error_docref(NULL, E_WARNING, "Unrecognized key '%s' found in the options array", ZSTR_VAL(key));
            }
        } else {
            php_error_docref(NULL, E_WARNING, "Numeric key found in the options array");
        }
    } ZEND_HASH_FOREACH_END();

    /* Array is not empty but no valid keys were found */
    if (found == 0 && zend_hash_num_elements(Z_ARRVAL_P(options)) > 0) {
        php_error_docref(NULL, E_WARNING, "No valid options were found in the given array");
    }
}

static void server_response_set_cookie(INTERNAL_FUNCTION_PARAMETERS, zend_bool url_encode)
{
    zval *response = getThis();
    zval *ptr;
    zval member = {0};
    zval cookie = {0};
    zval *arr;

    zend_string *name;
    zend_string *value = NULL;
    zval *expires_or_options = NULL;
    zend_long expires = 0;
    zend_string *path = NULL;
    zend_string *domain = NULL;
    zend_bool secure = 0;
    zend_bool httponly = 0;
    zend_string *samesite = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 7)
        Z_PARAM_STR(name)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(value)
        Z_PARAM_ZVAL(expires_or_options)
        Z_PARAM_STR(path)
        Z_PARAM_STR(domain)
        Z_PARAM_BOOL(secure)
        Z_PARAM_BOOL(httponly)
    ZEND_PARSE_PARAMETERS_END();

    if (expires_or_options) {
        if (Z_TYPE_P(expires_or_options) == IS_ARRAY) {
            if (UNEXPECTED(ZEND_NUM_ARGS() > 3)) {
                zend_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Cannot pass arguments after the options array");
                return;
            }
            php_head_parse_cookie_options_array(expires_or_options, &expires, &path, &domain, &secure, &httponly, &samesite);
        } else {
            expires = zval_get_long(expires_or_options);
        }
    }

    if (EG(exception)) {
        return;
    }

    // Read property pointer
    ZVAL_STRING(&member, "cookies");
    ptr = Z_OBJ_HT_P(response)->get_property_ptr_ptr(response, &member, BP_VAR_RW, NULL);

    if( !ptr ) {
        // fall-through
    } else if( Z_TYPE_P(ptr) != IS_ARRAY ) {
        convert_to_array(ptr);
        //ptr = NULL;
    }

    // Make cookie info array
    array_init_size(&cookie, 8);

    if( value ) {
        add_assoc_stringl_ex(&cookie, ZEND_STRL("value"), ZSTR_VAL(value), ZSTR_LEN(value));
    } else {
        add_assoc_stringl_ex(&cookie, ZEND_STRL("value"), ZEND_STRL(""));
    }

    add_assoc_long_ex(&cookie, ZEND_STRL("expires"), expires);

    if( path ) {
        add_assoc_str_ex(&cookie, ZEND_STRL("path"), path);
    } else {
        add_assoc_stringl_ex(&cookie, ZEND_STRL("path"), ZEND_STRL(""));
    }

    if( domain ) {
        add_assoc_str_ex(&cookie, ZEND_STRL("domain"), domain);
    } else {
        add_assoc_stringl_ex(&cookie, ZEND_STRL("domain"), ZEND_STRL(""));
    }

    add_assoc_bool_ex(&cookie, ZEND_STRL("secure"), secure);
    add_assoc_bool_ex(&cookie, ZEND_STRL("httponly"), httponly);

    if( samesite ) {
        add_assoc_str_ex(&cookie, ZEND_STRL("samesite"), samesite);
    } else {
        add_assoc_stringl_ex(&cookie, ZEND_STRL("samesite"), ZEND_STRL(""));
    }

    add_assoc_bool_ex(&cookie, ZEND_STRL("url_encode"), url_encode);

    // Update property
    if( ptr ) {
        add_assoc_zval_ex(ptr, ZSTR_VAL(name), ZSTR_LEN(name), &cookie);
    } else {
        zend_update_property(ServerResponse_ce_ptr, response, ZEND_STRL("cookies"), &cookie);
    }

    // Cleanup
    zval_ptr_dtor(&member);

    if (expires_or_options && Z_TYPE_P(expires_or_options) == IS_ARRAY) {
        if (path) {
            zend_string_release(path);
        }
        if (domain) {
            zend_string_release(domain);
        }
        if (samesite) {
            zend_string_release(samesite);
        }
    }

    RETURN_TRUE;
}

PHP_METHOD(ServerResponse, setCookie)
{
    return server_response_set_cookie(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} ServerResponse::setCookie */

/* {{{ proto void ServerResponse::setRawCookie(string name [, string value [, int expires [, string path [, string domain [, bool secure[, bool httponly]]]]]]) */
PHP_METHOD(ServerResponse, setRawCookie)
{
    return server_response_set_cookie(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} ServerResponse::setRawCookie */

/* {{{ proto void ServerResponse::unsetCookie(string $name) */
static void server_response_unset_cookie(zval *response, zend_string *name)
{
    zval member;
    zval *prop_ptr;

    // Read property pointer
    if( !Z_OBJ_HT_P(response)->get_property_ptr_ptr ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "ServerResponse::unsetCookie requires get_property_ptr_ptr");
        return;
    }

    ZVAL_STRING(&member, "cookies");
    prop_ptr = Z_OBJ_HT_P(response)->get_property_ptr_ptr(response, &member, BP_VAR_RW, NULL);
    if( !prop_ptr || Z_TYPE_P(prop_ptr) != IS_ARRAY ) {
        return;
    }

    zend_hash_del(Z_ARRVAL_P(prop_ptr), name);
}

PHP_METHOD(ServerResponse, unsetCookie)
{
    zend_string *name;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END();

    server_response_unset_cookie(getThis(), name);
}
/* }}} ServerResponse::unsetCookie */

/* {{{ proto void ServerResponse::unsetCookies() */
PHP_METHOD(ServerResponse, unsetCookies)
{
    zval *_this_zval = getThis();

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property_null(ServerResponse_ce_ptr, _this_zval, ZEND_STRL("cookies"));
}
/* }}} ServerResponse::unsetHeaders */

/* {{{ proto mixed ServerResponse::getContent() */
static zval *server_response_get_content(zval *response)
{
    return zend_read_property(ServerResponse_ce_ptr, response, ZEND_STRL("content"), 0, NULL);
}

PHP_METHOD(ServerResponse, getContent)
{
    zval *_this_zval = getThis();

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    RETVAL_ZVAL(server_response_get_content(_this_zval), 1, 0);
}
/* }}} ServerResponse::getContent */

/* {{{ proto void ServerResponse::setContent(mixed $content) */
PHP_METHOD(ServerResponse, setContent)
{
    zval *_this_zval = getThis();
    zval *content;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(content)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property(ServerResponse_ce_ptr, _this_zval, ZEND_STRL("content"), content);
}
/* }}} ServerResponse::setContent */

/* {{{ proto void ServerResponse::addHeaderCallback(callable $callback) */
PHP_METHOD(ServerResponse, addHeaderCallback)
{
    zval *callback_func;
    zval *_this_zval = getThis();
    zval member;
    zval *prop_ptr;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(callback_func)
    ZEND_PARSE_PARAMETERS_END();

    if( Z_TYPE_P(callback_func) == IS_NULL || !zend_is_callable(callback_func, 0, NULL) ) {
        RETURN_FALSE;
    }

    // Read property pointer
    if( !Z_OBJ_HT_P(_this_zval)->get_property_ptr_ptr ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "ServerResponse::addHeaderCallback requires get_property_ptr_ptr");
        return;
    }

    ZVAL_STRING(&member, "callbacks");
    prop_ptr = Z_OBJ_HT_P(_this_zval)->get_property_ptr_ptr(_this_zval, &member, BP_VAR_RW, NULL);
    if( !prop_ptr || Z_TYPE_P(prop_ptr) != IS_ARRAY ) {
        convert_to_array(prop_ptr);
    }

    // Append callback
    zval tmp = {0};
    ZVAL_ZVAL(&tmp, callback_func, 1, 0);
    add_next_index_zval(prop_ptr, &tmp);

    // Cleanup
    zval_ptr_dtor(&member);
}
/* }}} ServerResponse::setHeaderCallbacks */

/* {{{ proto void ServerResponse::setHeaderCallbacks(array $callbacks) */
PHP_METHOD(ServerResponse, setHeaderCallbacks)
{
    zval *callbacks;
    zval *callback;
    zval *_this_zval = getThis();
    zval arr;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(callbacks)
    ZEND_PARSE_PARAMETERS_END();

    // Reset callbacks property
    array_init(&arr);
    zend_update_property(ServerResponse_ce_ptr, _this_zval, ZEND_STRL("callbacks"), &arr);

    // Forward each item to addHeaderCallback
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(callbacks), callback) {
        zend_call_method_with_1_params(_this_zval, NULL, NULL, "addHeaderCallback", NULL, callback);
    } ZEND_HASH_FOREACH_END();
}
/* }}} ServerResponse::setHeaderCallbacks */

/* {{{ proto callback ServerResponse::getHeaderCallbacks() */
static zval *server_response_get_header_callbacks(zval *response)
{
    return zend_read_property(ServerResponse_ce_ptr, response, ZEND_STRL("callbacks"), 0, NULL);
}

PHP_METHOD(ServerResponse, getHeaderCallbacks)
{
    zval *_this_zval = getThis();

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    RETVAL_ZVAL(server_response_get_header_callbacks(_this_zval), 1, 0);
}
/* }}} ServerResponse::getHeaderCallback */

/* {{{ ServerResponse methods */
static zend_function_entry ServerResponse_methods[] = {
    PHP_ME(ServerResponse, setVersion, ServerResponse_setVersion_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, getVersion, ServerResponse_getVersion_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, setCode, ServerResponse_setCode_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, getCode, ServerResponse_getCode_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, setHeader, ServerResponse_addHeader_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, addHeader, ServerResponse_setHeader_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, unsetHeader, ServerResponse_unsetHeader_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, unsetHeaders, ServerResponse_unsetHeaders_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, getHeaders, ServerResponse_getHeaders_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, setCookie, ServerResponse_setCookie_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, setRawCookie, ServerResponse_setCookie_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, unsetCookie, ServerResponse_unsetCookie_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, unsetCookies, ServerResponse_unsetCookies_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, getCookies, ServerResponse_getCookies_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, setContent, ServerResponse_setContent_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, getContent, ServerResponse_getContent_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, setHeaderCallbacks, ServerResponse_setHeaderCallbacks_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, addHeaderCallback, ServerResponse_addHeaderCallback_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, getHeaderCallbacks, ServerResponse_getHeaderCallbacks_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_FE_END
};
/* }}} ServerResponse methods */

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(serverresponse)
{
    zend_class_entry ServerResponse_ce;

    INIT_CLASS_ENTRY(ServerResponse_ce, "ServerResponse", ServerResponse_methods);
    ServerResponse_ce_ptr = zend_register_internal_class(&ServerResponse_ce);

    zend_declare_property_null(ServerResponse_ce_ptr, ZEND_STRL("version"), ZEND_ACC_PRIVATE);
    zend_declare_property_null(ServerResponse_ce_ptr, ZEND_STRL("code"), ZEND_ACC_PRIVATE);
    zend_declare_property_null(ServerResponse_ce_ptr, ZEND_STRL("headers"), ZEND_ACC_PRIVATE);
    zend_declare_property_null(ServerResponse_ce_ptr, ZEND_STRL("cookies"), ZEND_ACC_PRIVATE);
    zend_declare_property_null(ServerResponse_ce_ptr, ZEND_STRL("content"), ZEND_ACC_PRIVATE);
    zend_declare_property_null(ServerResponse_ce_ptr, ZEND_STRL("callbacks"), ZEND_ACC_PRIVATE);

    return SUCCESS;
}

/* ServerResponseSender *********************************************************** */

/* {{{ Argument Info */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponseSender_send_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, response, ServerResponse, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponseSender_runHeaderCallbacks_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, response, ServerResponse, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponseSender_sendStatus_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, response, ServerResponse, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponseSender_sendHeaders_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, response, ServerResponse, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponseSender_sendCookies_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, response, ServerResponse, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponseSender_sendContent_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, response, ServerResponse, 0)
ZEND_END_ARG_INFO()

/* }}} Argument Info */

/* {{{ proto void ServerResponseSender::runHeaderCallbacks() */
static void server_response_sender_run_header_callbacks(zval *response)
{
    zval *callbacks;
    zval *callback;

    callbacks = server_response_get_header_callbacks(response);

    if( !callbacks || Z_TYPE_P(callbacks) != IS_ARRAY ) {
        return;
    }

    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(callbacks), callback) {

        int error;
        char *callback_error = NULL;
        zval retval = {0};
        zval params[1] = {{0}};
        zend_fcall_info fci = empty_fcall_info;
        zend_fcall_info_cache fcic = empty_fcall_info_cache;

        if( zend_fcall_info_init(callback, 0, &fci, &fcic, NULL, &callback_error) == SUCCESS ) {
            ZVAL_ZVAL(&params[0], response, 1, 0);
            fci.retval = &retval;
            fci.params = &params;
            fci.param_count = 1;

            error = zend_call_function(&fci, &fcic);
            if (error == FAILURE) {
                goto callback_failed;
            } else {
                zval_ptr_dtor(&retval);
            }

            zval_ptr_dtor(&params[0]);
        } else {
            callback_failed:
            zend_throw_exception_ex(spl_ce_RuntimeException, 0, "Could not call a header callback");
        }

        if (callback_error) {
            efree(callback_error);
        }
    } ZEND_HASH_FOREACH_END();
}

PHP_METHOD(ServerResponseSender, runHeaderCallbacks)
{
    zval *response;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(response, ServerResponse_ce_ptr)
    ZEND_PARSE_PARAMETERS_END();

    server_response_sender_run_header_callbacks(response);
}
/* }}} ServerResponseSender::runHeaderCallbacks */

/* {{{ proto void ServerResponseSender::sendStatus() */
static void server_response_sender_send_status(zval *response)
{
    sapi_header_line ctr = {0};
    zval *tmp;
    smart_str buf = {0};

    // Make code
    tmp = server_response_get_code(response);
    if( tmp ) {
        ctr.response_code = zval_get_long(tmp);
    } else {
        ctr.response_code = 200;
    }

    // Make header
    smart_str_appendl_ex(&buf, ZEND_STRL("HTTP/"), 0);

    tmp = server_response_get_code(response);
    if( tmp ) {
        convert_to_string(tmp);
        smart_str_append_ex(&buf, Z_STR_P(tmp), 0);
    } else {
        smart_str_appendl_ex(&buf, ZEND_STRL("1.1"), 0);
    }

    smart_str_appendc_ex(&buf, ' ', 0);
    smart_str_append_long_ex(&buf, ctr.response_code, 0);
    smart_str_0(&buf);

    ctr.line = ZSTR_VAL(buf.s);
    ctr.line_len = ZSTR_LEN(buf.s);

    sapi_header_op(SAPI_HEADER_REPLACE, &ctr);

    smart_str_free(&buf);
}

PHP_METHOD(ServerResponseSender, sendStatus)
{
    zval *response;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(response, ServerResponse_ce_ptr)
    ZEND_PARSE_PARAMETERS_END();

    server_response_sender_send_status(response);
}
/* }}} ServerResponseSender::sendStatus */

/* {{{ proto void ServerResponseSender::sendHeaders() */
static inline void server_response_sender_send_header(zend_string *header, zval *value)
{
    sapi_header_line ctr = {0};
    smart_str buf = {0};

    smart_str_append(&buf, header);
    smart_str_appendl_ex(&buf, ZEND_STRL(": "), 0);
    smart_str_appendz(&buf, value);
    smart_str_0(&buf);

    ctr.response_code = 0;
    ctr.line = ZSTR_VAL(buf.s);
    ctr.line_len = ZSTR_LEN(buf.s);
    sapi_header_op(SAPI_HEADER_ADD, &ctr);

    smart_str_free(&buf);
}

static void server_response_sender_send_headers(zval *response)
{
    zval *headers;
    zend_ulong index;
    zend_string *label;
    zval *value;

    headers = server_response_get_headers(response);
    if( !headers || Z_TYPE_P(headers) != IS_ARRAY ) {
        return;
    }

    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(headers), index, label, value) {
        if( label ) {
            server_response_sender_send_header(label, value);
        }
    } ZEND_HASH_FOREACH_END();
}

PHP_METHOD(ServerResponseSender, sendHeaders)
{
    zval *response;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(response, ServerResponse_ce_ptr)
    ZEND_PARSE_PARAMETERS_END();

    server_response_sender_send_headers(response);
}
/* }}} ServerResponseSender::sendHeaders */

/* {{{ proto void ServerResponseSender::sendCookies() */
static inline void server_response_sender_send_cookie(zend_string *name, zval *arr)
{
    zval *tmp;
    zend_string *value;
    zend_long expires = 0;
    zend_string *path = NULL;
    zend_string *domain = NULL;
    zend_bool secure = 0;
    zend_bool httponly = 0;
    zend_string *samesite = NULL;
    zend_bool url_encode = 0;

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(arr), ZEND_STRL("value"))) ) {
        value = zval_get_string(tmp);
    } else {
        return;
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(arr), ZEND_STRL("expires"))) ) {
        expires = zval_get_long(tmp);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(arr), ZEND_STRL("path"))) ) {
        path = zval_get_string(tmp);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(arr), ZEND_STRL("domain"))) ) {
        domain = zval_get_string(tmp);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(arr), ZEND_STRL("secure"))) ) {
        secure = zval_is_true(tmp);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(arr), ZEND_STRL("httponly"))) ) {
        httponly = zval_is_true(tmp);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(arr), ZEND_STRL("samesite"))) ) {
        samesite = zval_get_string(tmp);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(arr), ZEND_STRL("url_encode"))) ) {
        url_encode = zval_is_true(tmp);
    }

    php_setcookie(name, value, expires, path, domain, secure, httponly, samesite, url_encode);
}

static void server_response_sender_send_cookies(zval *response)
{
    zval *cookies;
    zend_string *key;
    zend_ulong index;
    zval *val;

    cookies = server_response_get_cookies(response);

    if( !cookies || Z_TYPE_P(cookies) != IS_ARRAY ) {
        return;
    }

    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(cookies), index, key, val) {
        if( key && Z_TYPE_P(val) == IS_ARRAY ) {
            server_response_sender_send_cookie(key, val);
        }
    } ZEND_HASH_FOREACH_END();

}

PHP_METHOD(ServerResponseSender, sendCookies)
{
    zval *response;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(response, ServerResponse_ce_ptr)
    ZEND_PARSE_PARAMETERS_END();

    server_response_sender_send_cookies(response);
}
/* }}} ServerResponseSender::sendCookies */

/* {{{ proto void ServerResponseSender::sendContent() */
static void server_response_sender_send_content(zval *response)
{
    zval *content;
    zend_string *content_str;
    php_stream *stream;
    char *error;
    zval func_name = {0};
    zval rv = {0};
    zval params[1] = {0};
    zval *return_value;

    content = server_response_get_content(response);
    if( !content ) {
        return;
    }

    if( Z_TYPE_P(content) == IS_OBJECT && zend_is_callable(content, 0, NULL) ) {
        ZVAL_STRING(&func_name, "__invoke");
        ZVAL_ZVAL(&params[0], response, 1, 0);
        call_user_function(&Z_OBJCE_P(content)->function_table, content, &func_name, &rv, 1, &params);
        zval_ptr_dtor(&func_name);
        zval_ptr_dtor(&params[0]);
        content = &rv;
    } else {
        Z_TRY_ADDREF_P(content);
    }

    switch( Z_TYPE_P(content) ) {
        case IS_UNDEF:
        case IS_NULL:
        case IS_FALSE:
            // do nothing
            break;

        case IS_RESOURCE:
            php_stream_from_res(stream, Z_RES_P(content)); // this macro can return
            php_stream_seek(stream, 0, SEEK_SET);
            php_stream_passthru(stream);
            break;

        case IS_STRING:
            php_output_write(Z_STRVAL_P(content), Z_STRLEN_P(content));
            break;

        default:
            content_str = zval_get_string(content);
            php_output_write(ZSTR_VAL(content_str), ZSTR_LEN(content_str));
            zend_string_release(content_str);
            break;
    }

    zval_ptr_dtor(content);
}

PHP_METHOD(ServerResponseSender, sendContent)
{
    zval *response;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(response, ServerResponse_ce_ptr)
    ZEND_PARSE_PARAMETERS_END();

    server_response_sender_send_content(response);

}
/* }}} ServerResponseSender::sendContent */

/* {{{ proto void ServerResponseSender::send() */
PHP_METHOD(ServerResponseSender, send)
{
    zval *response;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(response, ServerResponse_ce_ptr)
    ZEND_PARSE_PARAMETERS_END();

    server_response_sender_run_header_callbacks(response);
    server_response_sender_send_status(response);
    server_response_sender_send_headers(response);
    server_response_sender_send_cookies(response);
    server_response_sender_send_content(response);
}
/* }}} ServerResponseSender::send */

/* {{{ ServerResponseSender methods */
static zend_function_entry ServerResponseSender_methods[] = {
    PHP_ME(ServerResponseSender, send, ServerResponseSender_send_args, ZEND_ACC_PUBLIC)
    PHP_ME(ServerResponseSender, runHeaderCallbacks, ServerResponseSender_runHeaderCallbacks_args, ZEND_ACC_PUBLIC)
    PHP_ME(ServerResponseSender, sendStatus, ServerResponseSender_sendStatus_args, ZEND_ACC_PUBLIC)
    PHP_ME(ServerResponseSender, sendHeaders, ServerResponseSender_sendHeaders_args, ZEND_ACC_PUBLIC)
    PHP_ME(ServerResponseSender, sendCookies, ServerResponseSender_sendCookies_args, ZEND_ACC_PUBLIC)
    PHP_ME(ServerResponseSender, sendContent, ServerResponseSender_sendContent_args, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
/* }}} ServerResponseSender methods */

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(serverresponsesender)
{
    zend_class_entry ServerResponseSender_ce;

    INIT_CLASS_ENTRY(ServerResponseSender_ce, "ServerResponseSender", ServerResponseSender_methods);
    ServerResponseSender_ce_ptr = zend_register_internal_class(&ServerResponseSender_ce);

    return SUCCESS;
}
/* }}} PHP_MINIT_FUNCTION */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: et sw=4 ts=4
 */
