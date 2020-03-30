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

#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_interfaces.h"
#include "Zend/zend_portability.h"
#include "Zend/zend_types.h"
#include "Zend/zend_smart_str.h"

#include "php_request.h"
#include "request_utils.h"

extern PHP_MINIT_FUNCTION(sapiupload);
extern PHP_MSHUTDOWN_FUNCTION(sapiupload);

extern void sapi_request_parse_forwarded(zval *return_value, const unsigned char *str, size_t len);

static PHP_MINIT_FUNCTION(sapirequest);
static PHP_MINIT_FUNCTION(sapiresponse);
static PHP_MINIT_FUNCTION(sapiresponseinterface);
static PHP_MINIT_FUNCTION(sapiresponsesender);
static PHP_MSHUTDOWN_FUNCTION(sapirequest);

PHP_REQUEST_API zend_class_entry *SapiRequest_ce_ptr;
PHP_REQUEST_API zend_class_entry *SapiResponse_ce_ptr;
PHP_REQUEST_API zend_class_entry *SapiResponseInterface_ce_ptr;
PHP_REQUEST_API zend_class_entry *SapiResponseSender_ce_ptr;

/* {{{ PHP_MINIT_FUNCTION */
static PHP_MINIT_FUNCTION(request)
{
    PHP_MINIT(sapirequest)(INIT_FUNC_ARGS_PASSTHRU);
    PHP_MINIT(sapiresponseinterface)(INIT_FUNC_ARGS_PASSTHRU); // must be before sapiresponse
    PHP_MINIT(sapiresponse)(INIT_FUNC_ARGS_PASSTHRU);
    PHP_MINIT(sapiresponsesender)(INIT_FUNC_ARGS_PASSTHRU);
    PHP_MINIT(sapiupload)(INIT_FUNC_ARGS_PASSTHRU);
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
    PHP_MSHUTDOWN(sapirequest)(SHUTDOWN_FUNC_ARGS_PASSTHRU);
    PHP_MSHUTDOWN(sapiupload)(SHUTDOWN_FUNC_ARGS_PASSTHRU);
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

/* {{{ sapi_request_normalize_header_name */
void sapi_request_normalize_header_name(char *key, size_t key_length)
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

static zend_string *sapi_request_normalize_header_name_ex(zend_string *in)
{
    zend_string *out = php_trim(in, ZEND_STRL(" \t\r\n\v"), 3);
    sapi_request_normalize_header_name(ZSTR_VAL(out), ZSTR_LEN(out));
    zend_string_forget_hash_val(out);
    zend_string_hash_val(out);
    return out;
}
/* }}} sapi_request_normalize_header_name */

/* SapiRequest ************************************************************ */

extern void sapi_request_parse_accept(zval *return_value, const unsigned char *str, size_t len);
extern void sapi_request_parse_content_type(zval *return_value, const unsigned char *str, size_t len);
extern void sapi_request_parse_digest_auth(zval *return_value, const unsigned char *str, size_t len);
extern void sapi_request_parse_x_forwarded_for(zval *return_value, const unsigned char *str, size_t len);
extern void sapi_request_parse_x_forwarded(zval *return_value, const unsigned char *str, size_t len);
extern void sapi_request_normalize_header_name(char *key, size_t key_length);


static zend_object_handlers SapiRequest_obj_handlers;
static HashTable SapiRequest_prop_handlers;


/* {{{ Argument Info */
ZEND_BEGIN_ARG_INFO_EX(SapiRequest_construct_args, 0, 0, 1)
    ZEND_ARG_ARRAY_INFO(0, globals, 0)
    ZEND_ARG_TYPE_INFO(0, content, IS_STRING, 1)
ZEND_END_ARG_INFO()
/* }}} Argument Info */

/* {{ sapi_request_detect_method */
static void sapi_request_detect_method(zval *return_value, zval *server)
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

/* {{{ sapi_request_is_secure */
static zend_bool sapi_request_is_secure(zval *server)
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

/* {{{ sapi_request_detect_url */
static inline const unsigned char *sapi_request_extract_port_from_host(const unsigned char *host, size_t len)
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

static inline zend_string *sapi_request_extract_host_from_server(zval *server)
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

static inline zend_long sapi_request_extract_port_from_server(zval *server, zend_string *host)
{
    zval *tmp;

    // Get port
    if( NULL != sapi_request_extract_port_from_host(ZSTR_VAL(host), ZSTR_LEN(host)) ) {
        // no need to extract
    } else if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("SERVER_PORT"))) ) {
        return zval_get_long(tmp);
    }

    return 0;
}

static inline zend_string *sapi_request_extract_uri_from_server(zval *server)
{
    zval *tmp;

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("REQUEST_URI"))) &&
        Z_TYPE_P(tmp) == IS_STRING ) {
        return Z_STR_P(tmp);
    }

    return NULL;
}

static zend_string *sapi_request_detect_url(zval *server)
{
    zend_string *host;
    zend_long port;
    zend_string *uri;
    smart_str buf = {0};
    zend_bool is_secure = sapi_request_is_secure(server);
    const char *fake_host = "___";

    host = sapi_request_extract_host_from_server(server);
    port = sapi_request_extract_port_from_server(server, host);
    uri = sapi_request_extract_uri_from_server(server);

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

/* {{{ sapi_request_normalize_headers */
static void sapi_request_normalize_headers(zval *return_value, zval *server)
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
            sapi_request_normalize_header_name(ZSTR_VAL(tmp), ZSTR_LEN(tmp));
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

/* {{{ sapi_request_normalize_files */
static void sapi_request_upload_from_spec(zval *return_value, zval *file);

static inline void sapi_request_copy_upload_key(zval *return_value, zval *nested, const char *key1, size_t key1_len, zend_ulong index2, zend_string *key2)
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

static inline void sapi_request_upload_from_nested(zval *return_value, zval *nested, zval *tmp_name)
{
    zend_string *key;
    zend_ulong index;
    zval tmp = {0};
    zval tmp2 = {0};

    array_init(return_value);

    ZEND_HASH_FOREACH_KEY(Z_ARRVAL_P(tmp_name), index, key) {
        array_init(&tmp);
        sapi_request_copy_upload_key(&tmp, nested, ZEND_STRL("error"), index, key);
        sapi_request_copy_upload_key(&tmp, nested, ZEND_STRL("name"), index, key);
        sapi_request_copy_upload_key(&tmp, nested, ZEND_STRL("size"), index, key);
        sapi_request_copy_upload_key(&tmp, nested, ZEND_STRL("tmp_name"), index, key);
        sapi_request_copy_upload_key(&tmp, nested, ZEND_STRL("type"), index, key);

        sapi_request_upload_from_spec(&tmp2, &tmp);
        if( key ) {
            add_assoc_zval_ex(return_value, ZSTR_VAL(key), ZSTR_LEN(key), &tmp2);
        } else {
            add_index_zval(return_value, index, &tmp2);
        }
    } ZEND_HASH_FOREACH_END();
}

static inline void sapi_request_copy_unnested_upload_key(zval *out, zval *in, const char *key, size_t key_len)
{
    zval *tmp = zend_hash_str_find(Z_ARRVAL_P(in), key, key_len);
    if (tmp) {
        ZVAL_ZVAL(out, tmp, 1, 0);
    } else {
        ZVAL_NULL(out);
    }
}

static void sapi_request_upload_from_spec(zval *return_value, zval *file)
{
    zval *tmp = zend_hash_str_find(Z_ARRVAL_P(file), ZEND_STRL("tmp_name"));
    if( tmp && Z_TYPE_P(tmp) == IS_ARRAY ) {
        sapi_request_upload_from_nested(return_value, file, tmp);
    } else {
        object_init_ex(return_value, SapiUpload_ce_ptr);

        zval params[5] = {0};
        sapi_request_copy_unnested_upload_key(&params[0], file, ZEND_STRL("name"));
        sapi_request_copy_unnested_upload_key(&params[1], file, ZEND_STRL("type"));
        sapi_request_copy_unnested_upload_key(&params[2], file, ZEND_STRL("size"));
        sapi_request_copy_unnested_upload_key(&params[3], file, ZEND_STRL("tmp_name"));
        sapi_request_copy_unnested_upload_key(&params[4], file, ZEND_STRL("error"));

        zval z_const = {0};
        ZVAL_STRING(&z_const, "__construct");

        zval rv = {0};
        call_user_function(&SapiUpload_ce_ptr->function_table, return_value, &z_const, &rv, 5, params);

        zval_dtor(&rv);
        zval_dtor(&z_const);

        zval_dtor(&params[0]);
        zval_dtor(&params[1]);
        zval_dtor(&params[2]);
        zval_dtor(&params[3]);
        zval_dtor(&params[4]);
    }
}

static void sapi_request_normalize_files(zval *return_value, zval *files)
{
    zend_string *key;
    zend_ulong index;
    zval *val;
    zval tmp = {0};

    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(files), index, key, val) {
        if( Z_TYPE_P(val) == IS_ARRAY ) {
            sapi_request_upload_from_spec(&tmp, val);
            if( key ) {
                add_assoc_zval_ex(return_value, ZSTR_VAL(key), ZSTR_LEN(key), &tmp);
            } else {
                add_index_zval(return_value, index, &tmp);
            }
        }
    } ZEND_HASH_FOREACH_END();
}
/* }}} */

/* {{{ sapi_request_obj_create */
static zend_object *sapi_request_obj_create(zend_class_entry *ce)
{
    zend_object *obj;

    obj = ecalloc(1, sizeof(*obj) + zend_object_properties_size(ce));
    zend_object_std_init(obj, ce);
    object_properties_init(obj, ce);
    obj->handlers = &SapiRequest_obj_handlers;

    return obj;
}
/* }}} */

/* {{{ sapi_request_clone_obj */
#if PHP_MAJOR_VERSION >= 8
static zend_object *sapi_request_clone_obj(zend_object *zobject)
{
    zend_object * new_obj = std_object_handlers.clone_obj(zobject);
    new_obj->handlers = &SapiRequest_obj_handlers;
    return new_obj;
}
#else
static zend_object *sapi_request_clone_obj(zval *zobject)
{
    zend_object * new_obj = std_object_handlers.clone_obj(zobject);
    new_obj->handlers = &SapiRequest_obj_handlers;
    return new_obj;
}
#endif
/* }}} */

/* {{{ sapi_request_assert_immutable */
static int sapi_request_value_is_immutable(zval *value)
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
                if( !sapi_request_value_is_immutable(val) ) {
                    return 0;
                }
            } ZEND_HASH_FOREACH_END();
            return 1;
        default:
            return 0;
    }
}

static void sapi_request_assert_immutable(zval *value, const char *desc, size_t desc_len)
{
    if( !sapi_request_value_is_immutable(value) ) {
        zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "All $%.*s values must be null, scalar, or array.", (int)desc_len, desc);
    }
}
/* }}} */

/* {{{ sapi_request_object_content_read_property */
#if PHP_MAJOR_VERSION >= 8
static zval *sapi_request_object_content_read_property(zend_object *object, zend_string *member, int type, void **cache_slot, zval *rv)
{
    zval *retval = request_readonly_read_property_handler(object, member, type, cache_slot, rv);
    php_stream *stream;
    zend_string *str;

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
#else
static zval *sapi_request_object_content_read_property(zval *object, zval *member, int type, void **cache_slot, zval *rv)
{
    zval *retval = request_readonly_read_property_handler(object, member, type, cache_slot, rv);
    php_stream *stream;
    zend_string *str;

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
#endif
/* }}} */

/* {{{ sapi_request_object_default_write_property */
#if PHP_MAJOR_VERSION >= 8
static zval *sapi_request_object_default_write_property(zend_object *object, zend_string *member, zval *value, void **cache_slot)
{
    return request_readonly_write_property_handler(SapiRequest_ce_ptr, object, member, value, cache_slot);
}
#elif PHP_MAJOR_VERSION >= 7 && PHP_MINOR_VERSION >= 4
static zval *sapi_request_object_default_write_property(zval *object, zval *member, zval *value, void **cache_slot)
{
    return request_readonly_write_property_handler(SapiRequest_ce_ptr, object, member, value, cache_slot);
}
#else
static void sapi_request_object_default_write_property(zval *object, zval *member, zval *value, void **cache_slot)
{
    request_readonly_write_property_handler(SapiRequest_ce_ptr, object, member, value, cache_slot);
}
#endif
/* }}} */

/* {{{ sapi_request_object_default_unset_property */
#if PHP_MAJOR_VERSION >= 8
static void sapi_request_object_default_unset_property(zend_object *object, zend_string *member, void **cache_slot)
{
    request_readonly_unset_property_handler(SapiRequest_ce_ptr, object, member, cache_slot);
}
#else
static void sapi_request_object_default_unset_property(zval *object, zval *member, void **cache_slot)
{
    request_readonly_unset_property_handler(SapiRequest_ce_ptr, object, member, cache_slot);
}
#endif
/* }}} */

/* {{{ sapi_request_object_has_property */
#if PHP_MAJOR_VERSION >= 8
static int sapi_request_object_has_property(zend_object *object, zend_string *member, int has_set_exists, void **cache_slot)
{
    return request_has_property_dispatcher(&SapiRequest_prop_handlers, object, member, has_set_exists, cache_slot);
}
#else
static int sapi_request_object_has_property(zval *object, zval *member, int has_set_exists, void **cache_slot)
{
    return request_has_property_dispatcher(&SapiRequest_prop_handlers, object, member, has_set_exists, cache_slot);
}
#endif
/* }}} */

/* {{{ sapi_request_object_read_property */
#if PHP_MAJOR_VERSION >= 8
static zval *sapi_request_object_read_property(zend_object *object, zend_string *member, int type, void **cache_slot, zval *rv)
{
    return request_read_property_dispatcher(&SapiRequest_prop_handlers, object, member, type, cache_slot, rv);
}
#else
static zval *sapi_request_object_read_property(zval *object, zval *member, int type, void **cache_slot, zval *rv)
{
    return request_read_property_dispatcher(&SapiRequest_prop_handlers, object, member, type, cache_slot, rv);
}
#endif
/* }}} */

/* {{{ sapi_request_object_write_property */
#if PHP_MAJOR_VERSION >= 8
static zval *sapi_request_object_write_property(zend_object *object, zend_string *member, zval *value, void **cache_slot)
{
    return request_write_property_dispatcher(&SapiRequest_prop_handlers, object, member, value, cache_slot);
}
#elif PHP_MAJOR_VERSION >= 7 && PHP_MINOR_VERSION >= 4
static zval *sapi_request_object_write_property(zval *object, zval *member, zval *value, void **cache_slot)
{
    return request_write_property_dispatcher(&SapiRequest_prop_handlers, object, member, value, cache_slot);
}
#else
static void sapi_request_object_write_property(zval *object, zval *member, zval *value, void **cache_slot)
{
    request_write_property_dispatcher(&SapiRequest_prop_handlers, object, member, value, cache_slot);
}
#endif
/* }}} */

/* {{{ sapi_request_object_unset_property */
#if PHP_MAJOR_VERSION >= 8
static void sapi_request_object_unset_property(zend_object *object, zend_string *member, void **cache_slot)
{
    return request_unset_property_dispatcher(&SapiRequest_prop_handlers, object, member, cache_slot);
}
#else
static void sapi_request_object_unset_property(zval *object, zval *member, void **cache_slot)
{
    request_unset_property_dispatcher(&SapiRequest_prop_handlers, object, member, cache_slot);
}
#endif
/* }}} */

/* {{{ sapi_request_object_get_property_ptr_ptr */
#if PHP_MAJOR_VERSION >= 8
static zval *sapi_request_object_get_property_ptr_ptr(zend_object *object, zend_string *name, int type, void **cache_slot)
{
    return NULL;
}
#else
static zval *sapi_request_object_get_property_ptr_ptr(zval *object, zval *name, int type, void **cache_slot)
{
    return NULL;
}
#endif
/* }}} */

/* {{{ register_prop_handlers */
static inline void register_default_prop_handlers(const char *name, size_t name_length)
{
    register_prop_handlers(
        &SapiRequest_prop_handlers,
        name,
        name_length,
        request_readonly_has_property_handler,
        request_readonly_read_property_handler,
        sapi_request_object_default_write_property,
        sapi_request_object_default_unset_property
    );
}
/* }}} */

/* {{{ proto SapiRequest::__construct([ array $globals ]) */
static inline void sapi_request_copy_global_prop(
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
        sapi_request_assert_immutable(tmp, glob_key, glob_key_length);
        if( EG(exception) ) {
            return;
        }
        // Update property value
        zend_update_property(SapiRequest_ce_ptr, obj, obj_key, obj_key_length, tmp);
        Z_TRY_ADDREF_P(tmp);
    }
}

static inline void sapi_request_init_array_prop(
    zval *obj,
    const char *obj_key,
    size_t obj_key_length
) {
    zval tmp = {0};
    array_init(&tmp);
    zend_update_property(SapiRequest_ce_ptr, obj, obj_key, obj_key_length, &tmp);
}

static inline void sapi_request_set_forwarded(zval *object, zval *server)
{
    zval forwardedFor;
    zval forwardedHost;
    zval forwardedProto;
    zval forwarded;
    zval *tmp;
    zend_string *tmp_str;

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_X_FORWARDED_FOR"))) ) {
        array_init(&forwardedFor);
        sapi_request_parse_x_forwarded_for(&forwardedFor, Z_STRVAL_P(tmp), Z_STRLEN_P(tmp));
        zend_update_property(SapiRequest_ce_ptr, object, ZEND_STRL("forwardedFor"), &forwardedFor);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_X_FORWARDED_HOST"))) ) {
        convert_to_string(tmp);
        tmp_str = php_trim(Z_STR_P(tmp), ZEND_STRL(" \t\r\n\v"), 3);
        ZVAL_STR(&forwardedHost, tmp_str);
        zend_update_property(SapiRequest_ce_ptr, object, ZEND_STRL("forwardedHost"), &forwardedHost);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_X_FORWARDED_PROTO"))) ) {
        convert_to_string(tmp);
        tmp_str = php_trim(Z_STR_P(tmp), ZEND_STRL(" \t\r\n\v"), 3);
        ZVAL_STR(&forwardedProto, tmp_str);
        zend_update_property(SapiRequest_ce_ptr, object, ZEND_STRL("forwardedProto"), &forwardedProto);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_FORWARDED"))) ) {
        array_init(&forwarded);
        sapi_request_parse_forwarded(&forwarded, Z_STRVAL_P(tmp), Z_STRLEN_P(tmp));
        zend_update_property(SapiRequest_ce_ptr, object, ZEND_STRL("forwarded"), &forwarded);
    }
}

static inline void sapi_request_set_url(zval *object, zval *server)
{
    zend_string *tmp;
    php_url *url;
    zval arr = {0};
    const char *fake_host = "___";

    tmp = sapi_request_detect_url(server);
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

    zend_update_property(SapiRequest_ce_ptr, object, ZEND_STRL("url"), &arr);

    php_url_free(url);
}

static inline void sapi_request_set_accept_by_name(zval *object, zval *server, const char *src, size_t src_length, const char *dest, size_t dest_length)
{
    zval val = {0};
    zval *tmp;

    array_init(&val);
    tmp = zend_hash_str_find(Z_ARRVAL_P(server), src, src_length);
    if( tmp && Z_TYPE_P(tmp) == IS_STRING ) {
        sapi_request_parse_accept(&val, Z_STRVAL_P(tmp), Z_STRLEN_P(tmp));
        zend_update_property(SapiRequest_ce_ptr, object, dest, dest_length, &val);
    }
}

static inline void sapi_request_parse_accept_language(zval *lang)
{
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

static inline void sapi_request_set_accept_language(zval *object, zval *server)
{
    zval val = {0};
    zval *tmp;

    array_init(&val);
    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_ACCEPT_LANGUAGE"))) ) {
        sapi_request_parse_accept(&val, Z_STRVAL_P(tmp), Z_STRLEN_P(tmp));
        sapi_request_parse_accept_language(&val);
        zend_update_property(SapiRequest_ce_ptr, object, ZEND_STRL("acceptLanguage"), &val);
    }
}

static inline void sapi_request_set_auth(zval *object, zval *server)
{
    zval *tmp;
    zval digest = {0};

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("PHP_AUTH_PW"))) ) {
        zend_update_property(SapiRequest_ce_ptr, object, ZEND_STRL("authPw"), tmp);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("PHP_AUTH_TYPE"))) ) {
        zend_update_property(SapiRequest_ce_ptr, object, ZEND_STRL("authType"), tmp);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("PHP_AUTH_USER"))) ) {
        zend_update_property(SapiRequest_ce_ptr, object, ZEND_STRL("authUser"), tmp);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("PHP_AUTH_DIGEST"))) ) {
        zend_string *str = zval_get_string(tmp);
        sapi_request_parse_digest_auth(&digest, ZSTR_VAL(str), ZSTR_LEN(str));
        zend_update_property(SapiRequest_ce_ptr, object, ZEND_STRL("authDigest"), &digest);
    }

}

static inline void sapi_request_set_content_length(zval *object, zval *length)
{
    zend_string *tmp = php_trim(Z_STR_P(length), ZEND_STRL("0123456789 \t\r\n\v"), 3);
    if( zend_string_equals_literal(tmp, "") ) {
        zend_string_release(tmp);
        convert_to_long(length);
        zend_update_property(SapiRequest_ce_ptr, object, ZEND_STRL("contentLength"), length);
    }
}

static inline void sapi_request_set_content(zval *object, zval *server)
{
    zval *tmp;
    zval zv = {0};
    zval contentType = {0};

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_CONTENT_MD5"))) ) {
        zend_update_property(SapiRequest_ce_ptr, object, ZEND_STRL("contentMd5"), tmp);
    }

    if( tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_CONTENT_LENGTH")) ) {
        sapi_request_set_content_length(object, tmp);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_CONTENT_TYPE"))) && Z_TYPE_P(tmp) == IS_STRING ) {
        sapi_request_parse_content_type(&contentType, Z_STRVAL_P(tmp), Z_STRLEN_P(tmp));
        if( Z_TYPE(contentType) == IS_ARRAY ) {
            // contentType
            tmp = zend_hash_str_find(Z_ARRVAL(contentType), ZEND_STRL("value"));
            if( tmp ) {
                zend_update_property(SapiRequest_ce_ptr, object, ZEND_STRL("contentType"), tmp);
            }
            // charset
            tmp = zend_hash_str_find(Z_ARRVAL(contentType), ZEND_STRL("charset"));
            if( tmp ) {
                zend_update_property(SapiRequest_ce_ptr, object, ZEND_STRL("contentCharset"), tmp);
            }
        }
    }
}

/* {{{ SapiRequest::__construct */
PHP_METHOD(SapiRequest, __construct)
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

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_ARRAY(globals)
        Z_PARAM_OPTIONAL
        Z_PARAM_ZVAL(content)
    ZEND_PARSE_PARAMETERS_END();

    _this_zval = getThis();

    // Check and update isInitialized property
    init = zend_read_property(SapiRequest_ce_ptr, _this_zval, ZEND_STRL("isInitialized"), 0, &rv);
    if( zend_is_true(init) ) {
        zend_string *ce_name = Z_OBJCE_P(_this_zval)->name;
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "%.*s::__construct() called after construction.", (int)ZSTR_LEN(ce_name), ZSTR_VAL(ce_name));
        return;
    }
    zend_update_property_bool(SapiRequest_ce_ptr, _this_zval, ZEND_STRL("isInitialized"), 1);

    // initialize array properties
    sapi_request_init_array_prop(_this_zval, ZEND_STRL("accept"));
    sapi_request_init_array_prop(_this_zval, ZEND_STRL("acceptCharset"));
    sapi_request_init_array_prop(_this_zval, ZEND_STRL("acceptEncoding"));
    sapi_request_init_array_prop(_this_zval, ZEND_STRL("acceptLanguage"));
    sapi_request_init_array_prop(_this_zval, ZEND_STRL("authDigest"));
    sapi_request_init_array_prop(_this_zval, ZEND_STRL("cookie"));
    sapi_request_init_array_prop(_this_zval, ZEND_STRL("files"));
    sapi_request_init_array_prop(_this_zval, ZEND_STRL("forwarded"));
    sapi_request_init_array_prop(_this_zval, ZEND_STRL("forwardedFor"));
    sapi_request_init_array_prop(_this_zval, ZEND_STRL("headers"));
    sapi_request_init_array_prop(_this_zval, ZEND_STRL("input"));
    sapi_request_init_array_prop(_this_zval, ZEND_STRL("query"));
    sapi_request_init_array_prop(_this_zval, ZEND_STRL("server"));
    sapi_request_init_array_prop(_this_zval, ZEND_STRL("uploads"));
    sapi_request_init_array_prop(_this_zval, ZEND_STRL("url"));

    // Copy superglobals
    sapi_request_copy_global_prop(_this_zval, ZEND_STRL("server"), globals, ZEND_STRL("_SERVER"));
    sapi_request_copy_global_prop(_this_zval, ZEND_STRL("cookie"), globals, ZEND_STRL("_COOKIE"));
    sapi_request_copy_global_prop(_this_zval, ZEND_STRL("files"),  globals, ZEND_STRL("_FILES"));
    sapi_request_copy_global_prop(_this_zval, ZEND_STRL("input"),  globals, ZEND_STRL("_POST"));
    sapi_request_copy_global_prop(_this_zval, ZEND_STRL("query"),  globals, ZEND_STRL("_GET"));

    // Check if previous step threw
    if( EG(exception) ) {
        return;
    }

    // Read back server property
    server = zend_read_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("server"), 0, &rv);

    // Internal setters that require server
    if( server && Z_TYPE_P(server) == IS_ARRAY ) {
        // headers
        sapi_request_normalize_headers(&headers, server);
        zend_update_property(SapiRequest_ce_ptr, _this_zval, ZEND_STRL("headers"), &headers);

        // method
        sapi_request_detect_method(&method, server);
        zend_update_property(SapiRequest_ce_ptr, _this_zval, ZEND_STRL("method"), &method);

        // forwarded
        sapi_request_set_forwarded(_this_zval, server);

        // url
        sapi_request_set_url(_this_zval, server);

        // accepts
        sapi_request_set_accept_by_name(_this_zval, server, ZEND_STRL("HTTP_ACCEPT"), ZEND_STRL("accept"));
        sapi_request_set_accept_by_name(_this_zval, server, ZEND_STRL("HTTP_ACCEPT_CHARSET"), ZEND_STRL("acceptCharset"));
        sapi_request_set_accept_by_name(_this_zval, server, ZEND_STRL("HTTP_ACCEPT_ENCODING"), ZEND_STRL("acceptEncoding"));
        sapi_request_set_accept_language(_this_zval, server);

        // auth
        sapi_request_set_auth(_this_zval, server);
        sapi_request_set_content(_this_zval, server);
    }

    // Read back files property
    files = zend_read_property(SapiRequest_ce_ptr, _this_zval, ZEND_STRL("files"), 0, &rv);

    if( files && Z_TYPE_P(files) == IS_ARRAY ) {
        array_init(&uploads);
        sapi_request_normalize_files(&uploads, files);
        zend_update_property(SapiRequest_ce_ptr, _this_zval, ZEND_STRL("uploads"), &uploads);
    }

    if (content && Z_TYPE_P(content) == IS_STRING) {
        zend_update_property(SapiRequest_ce_ptr, _this_zval, ZEND_STRL("content"), content);
    }
}
/* }}} SapiRequest::__construct */

/* {{{ SapiRequest methods */
static zend_function_entry SapiRequest_methods[] = {
    PHP_ME(SapiRequest, __construct, SapiRequest_construct_args, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
/* }}} SapiRequest methods */

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(sapirequest)
{
    zend_class_entry ce;

    zend_hash_init(&SapiRequest_prop_handlers, 0, NULL, NULL, 1);

    memcpy(&SapiRequest_obj_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    SapiRequest_obj_handlers.has_property = sapi_request_object_has_property;
    SapiRequest_obj_handlers.read_property = sapi_request_object_read_property;
    SapiRequest_obj_handlers.write_property = sapi_request_object_write_property;
    SapiRequest_obj_handlers.unset_property = sapi_request_object_unset_property;
    SapiRequest_obj_handlers.get_property_ptr_ptr = sapi_request_object_get_property_ptr_ptr;
    SapiRequest_obj_handlers.clone_obj = sapi_request_clone_obj;

    INIT_CLASS_ENTRY(ce, "SapiRequest", SapiRequest_methods);
    SapiRequest_ce_ptr = zend_register_internal_class(&ce);
    SapiRequest_ce_ptr->create_object = sapi_request_obj_create;

    zend_declare_property_bool(SapiRequest_ce_ptr, ZEND_STRL("isInitialized"), 0, ZEND_ACC_PRIVATE);

    zend_declare_property_null(SapiRequest_ce_ptr, ZEND_STRL("accept"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("accept"));
    zend_declare_property_null(SapiRequest_ce_ptr, ZEND_STRL("acceptCharset"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("acceptCharset"));
    zend_declare_property_null(SapiRequest_ce_ptr, ZEND_STRL("acceptEncoding"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("acceptEncoding"));
    zend_declare_property_null(SapiRequest_ce_ptr, ZEND_STRL("acceptLanguage"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("acceptLanguage"));
    zend_declare_property_null(SapiRequest_ce_ptr, ZEND_STRL("authDigest"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("authDigest"));
    zend_declare_property_null(SapiRequest_ce_ptr, ZEND_STRL("authPw"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("authPw"));
    zend_declare_property_null(SapiRequest_ce_ptr, ZEND_STRL("authType"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("authType"));
    zend_declare_property_null(SapiRequest_ce_ptr, ZEND_STRL("authUser"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("authUser"));
    zend_declare_property_null(SapiRequest_ce_ptr, ZEND_STRL("content"), ZEND_ACC_PUBLIC);
    register_prop_handlers(
        &SapiRequest_prop_handlers,
        ZEND_STRL("content"),
        request_readonly_has_property_handler,
        sapi_request_object_content_read_property,
        sapi_request_object_default_write_property,
        sapi_request_object_default_unset_property
    );
    zend_declare_property_null(SapiRequest_ce_ptr, ZEND_STRL("contentCharset"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("contentCharset"));
    zend_declare_property_null(SapiRequest_ce_ptr, ZEND_STRL("contentLength"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("contentLength"));
    zend_declare_property_null(SapiRequest_ce_ptr, ZEND_STRL("contentMd5"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("contentMd5"));
    zend_declare_property_null(SapiRequest_ce_ptr, ZEND_STRL("contentType"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("contentType"));
    zend_declare_property_null(SapiRequest_ce_ptr, ZEND_STRL("cookie"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("cookie"));
    zend_declare_property_null(SapiRequest_ce_ptr, ZEND_STRL("files"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("files"));
    zend_declare_property_null(SapiRequest_ce_ptr, ZEND_STRL("forwarded"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("forwarded"));
    zend_declare_property_null(SapiRequest_ce_ptr, ZEND_STRL("forwardedFor"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("forwardedFor"));
    zend_declare_property_null(SapiRequest_ce_ptr, ZEND_STRL("forwardedHost"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("forwardedHost"));
    zend_declare_property_null(SapiRequest_ce_ptr, ZEND_STRL("forwardedProto"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("forwardedProto"));
    zend_declare_property_null(SapiRequest_ce_ptr, ZEND_STRL("headers"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("headers"));
    zend_declare_property_null(SapiRequest_ce_ptr, ZEND_STRL("input"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("input"));
    zend_declare_property_null(SapiRequest_ce_ptr, ZEND_STRL("method"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("method"));
    zend_declare_property_null(SapiRequest_ce_ptr, ZEND_STRL("query"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("query"));
    zend_declare_property_null(SapiRequest_ce_ptr, ZEND_STRL("server"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("server"));
    zend_declare_property_null(SapiRequest_ce_ptr, ZEND_STRL("uploads"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("uploads"));
    zend_declare_property_null(SapiRequest_ce_ptr, ZEND_STRL("url"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("url"));
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION */
PHP_MSHUTDOWN_FUNCTION(sapirequest)
{
    zend_hash_destroy(&SapiRequest_prop_handlers);
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
    smart_str_appendz_ex(dest, zv, 0);
}

/* SapiResponseInterface ******************************************************** */

/* {{{ SapiResponseInterface methods */
static zend_function_entry SapiResponseInterface_methods[] = {
    PHP_ABSTRACT_ME(SapiResponse, setVersion, SapiResponseInterface_setVersion_args)
    PHP_ABSTRACT_ME(SapiResponse, getVersion, SapiResponseInterface_getVersion_args)
    PHP_ABSTRACT_ME(SapiResponse, setCode, SapiResponseInterface_setCode_args)
    PHP_ABSTRACT_ME(SapiResponse, getCode, SapiResponseInterface_getCode_args)
    PHP_ABSTRACT_ME(SapiResponse, addHeader, SapiResponseInterface_addHeader_args)
    PHP_ABSTRACT_ME(SapiResponse, setHeader, SapiResponseInterface_setHeader_args)
    PHP_ABSTRACT_ME(SapiResponse, unsetHeader, SapiResponseInterface_unsetHeader_args)
    PHP_ABSTRACT_ME(SapiResponse, getHeader, SapiResponseInterface_getHeader_args)
    PHP_ABSTRACT_ME(SapiResponse, hasHeader, SapiResponseInterface_hasHeader_args)
    PHP_ABSTRACT_ME(SapiResponse, unsetHeaders, SapiResponseInterface_unsetHeaders_args)
    PHP_ABSTRACT_ME(SapiResponse, getHeaders, SapiResponseInterface_getHeaders_args)
    PHP_ABSTRACT_ME(SapiResponse, getCookie, SapiResponseInterface_getCookie_args)
    PHP_ABSTRACT_ME(SapiResponse, hasCookie, SapiResponseInterface_hasCookie_args)
    PHP_ABSTRACT_ME(SapiResponse, setCookie, SapiResponseInterface_setCookie_args)
    PHP_ABSTRACT_ME(SapiResponse, setRawCookie, SapiResponseInterface_setCookie_args)
    PHP_ABSTRACT_ME(SapiResponse, unsetCookie, SapiResponseInterface_unsetCookie_args)
    PHP_ABSTRACT_ME(SapiResponse, unsetCookies, SapiResponseInterface_unsetCookies_args)
    PHP_ABSTRACT_ME(SapiResponse, getCookies, SapiResponseInterface_getCookies_args)
    PHP_ABSTRACT_ME(SapiResponse, setContent, SapiResponseInterface_setContent_args)
    PHP_ABSTRACT_ME(SapiResponse, getContent, SapiResponseInterface_getContent_args)
    PHP_ABSTRACT_ME(SapiResponse, setHeaderCallbacks, SapiResponseInterface_setHeaderCallbacks_args)
    PHP_ABSTRACT_ME(SapiResponse, addHeaderCallback, SapiResponseInterface_addHeaderCallback_args)
    PHP_ABSTRACT_ME(SapiResponse, getHeaderCallbacks, SapiResponseInterface_getHeaderCallbacks_args)
    PHP_FE_END
};
/* }}} SapiResponseInterface methods */

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(sapiresponseinterface)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "SapiResponseInterface", SapiResponseInterface_methods);
    SapiResponseInterface_ce_ptr = zend_register_internal_interface(&ce);

    return SUCCESS;
}

/* SapiResponse *********************************************************** */

/* {{{ proto string SapiResponse::getVersion() */
static zval *sapi_response_get_version(zval *response)
{
    return zend_read_property(SapiResponse_ce_ptr, response, ZEND_STRL("version"), 0, NULL);
}

PHP_METHOD(SapiResponse, getVersion)
{
    zval *_this_zval = getThis();

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    RETVAL_ZVAL(sapi_response_get_version(_this_zval), 1, 0);
}
/* }}} SapiResponse::getVersion */

/* {{{ proto SapiResponseInterface SapiResponse::setVersion(string $version) */
PHP_METHOD(SapiResponse, setVersion)
{
    zval *_this_zval = getThis();
    zend_string *version;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(version)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property_str(SapiResponse_ce_ptr, _this_zval, ZEND_STRL("version"), version);

    RETURN_ZVAL(_this_zval, 1, 0);
}
/* }}} SapiResponse::setVersion */

/* {{{ proto int SapiResponse::getCode() */
static zval *sapi_response_get_code(zval *response)
{
    return zend_read_property(SapiResponse_ce_ptr, response, ZEND_STRL("code"), 0, NULL);
}

PHP_METHOD(SapiResponse, getCode)
{
    zval *_this_zval = getThis();

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    RETVAL_ZVAL(sapi_response_get_code(_this_zval), 1, 0);
}
/* }}} SapiResponse::getCode */

/* {{{ proto SapiResponseInterface SapiResponse::setCode(int $version) */
PHP_METHOD(SapiResponse, setCode)
{
    zval *_this_zval = getThis();
    zend_long code;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(code)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property_long(SapiResponse_ce_ptr, _this_zval, ZEND_STRL("code"), code);

    RETURN_ZVAL(_this_zval, 1, 0);
}
/* }}} SapiResponse::setCode */

/* {{{ proto array SapiResponse::getHeaders() */
static zval *sapi_response_get_headers(zval *response)
{
    return zend_read_property(SapiResponse_ce_ptr, response, ZEND_STRL("headers"), 0, NULL);
}

PHP_METHOD(SapiResponse, getHeaders)
{
    zval *_this_zval = getThis();

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    RETVAL_ZVAL(sapi_response_get_headers(_this_zval), 1, 0);
}
/* }}} SapiResponse::getHeaders */

/* {{{ proto SapiResponseInterface SapiResponse::setHeader(string $label, string $value) */
static void sapi_response_set_header(zval *response, zend_string *label, zend_string *value, zend_bool replace)
{
    zval *prop_ptr;
    zend_string *normal_label;
    zend_string *value_str;
    zval *prev_header = NULL;
    smart_str buf = {0};

    // Read property pointer
    if( !Z_OBJ_HT_P(response)->get_property_ptr_ptr ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "SapiResponse::setHeader requires get_property_ptr_ptr");
        return;
    }

    do {
#if PHP_MAJOR_VERSION >= 8
        zend_string *member = zend_string_init(ZEND_STRL("headers"), 0);
        prop_ptr = Z_OBJ_HT_P(response)->get_property_ptr_ptr(Z_OBJ_P(response), member, BP_VAR_RW, NULL);
        zend_string_release(member);
#else
        zval member = {0};
        ZVAL_STRING(&member, "headers");
        prop_ptr = Z_OBJ_HT_P(response)->get_property_ptr_ptr(response, &member, BP_VAR_RW, NULL);
        zval_ptr_dtor(&member);
#endif
    } while (0);

    if( !prop_ptr || Z_TYPE_P(prop_ptr) != IS_ARRAY ) {
        convert_to_array(prop_ptr);
    }

    // Normalize label
    normal_label = sapi_request_normalize_header_name_ex(label);

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

PHP_METHOD(SapiResponse, setHeader)
{
    zval *_this_zval = getThis();
    zend_string *label;
    zend_string *value;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(label)
        Z_PARAM_STR(value)
    ZEND_PARSE_PARAMETERS_END();

    sapi_response_set_header(_this_zval, label, value, 1);

    RETURN_ZVAL(_this_zval, 1, 0);
}
/* }}} SapiResponse::setHeader */

/* {{{ proto SapiResponseInterface SapiResponse::addHeader(string $label, string $value) */
PHP_METHOD(SapiResponse, addHeader)
{
    zval *_this_zval = getThis();
    zend_string *label;
    zend_string *value;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(label)
        Z_PARAM_STR(value)
    ZEND_PARSE_PARAMETERS_END();

    sapi_response_set_header(_this_zval, label, value, 0);

    RETURN_ZVAL(_this_zval, 1, 0);
}
/* }}} SapiResponse::addHeader */

/* {{{ proto SapiResponseInterfae SapiResponse::unsetHeader(string $label) */
static void sapi_response_unset_header(zval *response, zend_string *label)
{
    zval *prop_ptr;
    zend_string *normal_label;

    // Read property pointer
    if( !Z_OBJ_HT_P(response)->get_property_ptr_ptr ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "SapiResponse::unsetHeader requires get_property_ptr_ptr");
        return;
    }

    do {
#if PHP_MAJOR_VERSION >= 8
        zend_string *member = zend_string_init(ZEND_STRL("headers"), 0);
        prop_ptr = Z_OBJ_HT_P(response)->get_property_ptr_ptr(Z_OBJ_P(response), member, BP_VAR_RW, NULL);
        zend_string_release(member);
#else
        zval member = {0};
        ZVAL_STRING(&member, "headers");
        prop_ptr = Z_OBJ_HT_P(response)->get_property_ptr_ptr(response, &member, BP_VAR_RW, NULL);
        zval_ptr_dtor(&member);
#endif
    } while (0);

    if( !prop_ptr || Z_TYPE_P(prop_ptr) != IS_ARRAY ) {
        return;
    }

    normal_label = sapi_request_normalize_header_name_ex(label);

    if( ZSTR_LEN(normal_label) ) {
        zend_hash_del(Z_ARRVAL_P(prop_ptr), normal_label);
    }

    zend_string_release(normal_label);
}

PHP_METHOD(SapiResponse, unsetHeader)
{
    zval *_this_zval = getThis();
    zend_string *label;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(label)
    ZEND_PARSE_PARAMETERS_END();

    sapi_response_unset_header(_this_zval, label);

    RETURN_ZVAL(_this_zval, 1, 0);
}
/* }}} SapiResponse::unsetHeader */

/* {{{ proto ?string SapiResponse::getHeader(string $label) */
PHP_METHOD(SapiResponse, getHeader)
{
    zval *_this_zval = getThis();
    zend_string *label;
    zend_string *normal_label;
    zval *headers;
    zval *retval;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(label)
    ZEND_PARSE_PARAMETERS_END();

    headers = zend_read_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("headers"), 0, NULL);
    if( !headers || Z_TYPE_P(headers) != IS_ARRAY ) {
        return;
    }

    normal_label = sapi_request_normalize_header_name_ex(label);

    retval = zend_hash_find(Z_ARRVAL_P(headers), normal_label);
    if( retval ) {
        RETVAL_ZVAL(retval, 1, 0);
    }

    zend_string_release(normal_label);
}
/* }}} SapiResponse::getHeader */

/* {{{ proto bool SapiResponse::hasHeader(string $label) */
PHP_METHOD(SapiResponse, hasHeader)
{
    zval *_this_zval = getThis();
    zend_string *label;
    zend_string *normal_label;
    zval *headers;
    zval *retval;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(label)
    ZEND_PARSE_PARAMETERS_END();

    headers = zend_read_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("headers"), 0, NULL);
    if( !headers || Z_TYPE_P(headers) != IS_ARRAY ) {
        RETURN_FALSE;
    }

    normal_label = sapi_request_normalize_header_name_ex(label);

    retval = zend_hash_find(Z_ARRVAL_P(headers), normal_label);
    if( retval ) {
        RETVAL_TRUE;
    } else {
        RETVAL_FALSE;
    }

    zend_string_release(normal_label);
}
/* }}} SapiResponse::hasHeader */

/* {{{ proto SapiResponseInterface SapiResponse::unsetHeaders() */
PHP_METHOD(SapiResponse, unsetHeaders)
{
    zval *_this_zval = getThis();

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property_null(SapiResponse_ce_ptr, _this_zval, ZEND_STRL("headers"));

    RETURN_ZVAL(_this_zval, 1, 0);
}
/* }}} SapiResponse::unsetHeaders */

/* {{{ proto ?array SapiResponse::getCookie(string $name) */
PHP_METHOD(SapiResponse, getCookie)
{
    zval *_this_zval = getThis();
    zend_string *name;
    zval *cookies;
    zval *retval;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END();

    cookies = zend_read_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("cookies"), 0, NULL);
    if( !cookies || Z_TYPE_P(cookies) != IS_ARRAY ) {
        return;
    }

    retval = zend_hash_find(Z_ARRVAL_P(cookies), name);
    if( retval ) {
        RETVAL_ZVAL(retval, 1, 0);
    }
}
/* }}} SapiResponse::getCookie */

/* {{{ proto array SapiResponse::getCookies() */
static zval *sapi_response_get_cookies(zval *response)
{
    return zend_read_property(SapiResponse_ce_ptr, response, ZEND_STRL("cookies"), 0, NULL);
}

PHP_METHOD(SapiResponse, getCookies)
{
    zval *_this_zval = getThis();

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    RETVAL_ZVAL(sapi_response_get_cookies(_this_zval), 1, 0);
}
/* }}} SapiResponse::getCookies */

/* {{{ proto bool SapiResponse::hasCookie(string $name) */
PHP_METHOD(SapiResponse, hasCookie)
{
    zval *_this_zval = getThis();
    zend_string *name;
    zval *cookies;
    zval *retval;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END();

    cookies = zend_read_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("cookies"), 0, NULL);
    if( !cookies || Z_TYPE_P(cookies) != IS_ARRAY ) {
        RETURN_FALSE;
    }

    retval = zend_hash_find(Z_ARRVAL_P(cookies), name);
    if( retval ) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}
/* }}} SapiResponse::hasCookie */

/* {{{ proto SapiResponseInterface SapiResponse::setCookie(string name [, string value [, int expires [, string path [, string domain [, bool secure[, bool httponly]]]]]]) */
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

static void sapi_response_set_cookie(INTERNAL_FUNCTION_PARAMETERS, zend_bool url_encode)
{
    zval *response = getThis();
    zval *ptr;
    zval cookie = {0};

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
    do {
#if PHP_MAJOR_VERSION >= 8
        zend_string *member = zend_string_init(ZEND_STRL("cookies"), 0);
        ptr = Z_OBJ_HT_P(response)->get_property_ptr_ptr(Z_OBJ_P(response), member, BP_VAR_RW, NULL);
        zend_string_release(member);
#else
        zval member = {0};
        ZVAL_STRING(&member, "cookies");
        ptr = Z_OBJ_HT_P(response)->get_property_ptr_ptr(response, &member, BP_VAR_RW, NULL);
        zval_ptr_dtor(&member);
#endif
    } while (0);

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
        zend_update_property(SapiResponse_ce_ptr, response, ZEND_STRL("cookies"), &cookie);
    }

    RETURN_ZVAL(response, 1, 0);
}

PHP_METHOD(SapiResponse, setCookie)
{
    sapi_response_set_cookie(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} SapiResponse::setCookie */

/* {{{ proto SapiResponseInterface SapiResponse::setRawCookie(string name [, string value [, int expires [, string path [, string domain [, bool secure[, bool httponly]]]]]]) */
PHP_METHOD(SapiResponse, setRawCookie)
{
    sapi_response_set_cookie(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} SapiResponse::setRawCookie */

/* {{{ proto SapiResponseInterface SapiResponse::unsetCookie(string $name) */
static void sapi_response_unset_cookie(zval *response, zend_string *name)
{
    zval *prop_ptr;

    // Read property pointer
    if( !Z_OBJ_HT_P(response)->get_property_ptr_ptr ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "SapiResponse::unsetCookie requires get_property_ptr_ptr");
        return;
    }

    do {
#if PHP_MAJOR_VERSION >= 8
        zend_string *member = zend_string_init(ZEND_STRL("cookies"), 0);
        prop_ptr = Z_OBJ_HT_P(response)->get_property_ptr_ptr(Z_OBJ_P(response), member, BP_VAR_RW, NULL);
        zend_string_release(member);
#else
        zval member = {0};
        ZVAL_STRING(&member, "cookies");
        prop_ptr = Z_OBJ_HT_P(response)->get_property_ptr_ptr(response, &member, BP_VAR_RW, NULL);
        zval_ptr_dtor(&member);
#endif
    } while (0);

    if( !prop_ptr || Z_TYPE_P(prop_ptr) != IS_ARRAY ) {
        return;
    }

    zend_hash_del(Z_ARRVAL_P(prop_ptr), name);
}

PHP_METHOD(SapiResponse, unsetCookie)
{
    zval *_this_zval = getThis();
    zend_string *name;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END();

    sapi_response_unset_cookie(_this_zval, name);

    RETURN_ZVAL(_this_zval, 1, 0);
}
/* }}} SapiResponse::unsetCookie */

/* {{{ proto SapiResponseInterface SapiResponse::unsetCookies() */
PHP_METHOD(SapiResponse, unsetCookies)
{
    zval *_this_zval = getThis();

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property_null(SapiResponse_ce_ptr, _this_zval, ZEND_STRL("cookies"));

    RETURN_ZVAL(_this_zval, 1, 0);
}
/* }}} SapiResponse::unsetHeaders */

/* {{{ proto mixed SapiResponse::getContent() */
static zval *sapi_response_get_content(zval *response)
{
    return zend_read_property(SapiResponse_ce_ptr, response, ZEND_STRL("content"), 0, NULL);
}

PHP_METHOD(SapiResponse, getContent)
{
    zval *_this_zval = getThis();

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    RETVAL_ZVAL(sapi_response_get_content(_this_zval), 1, 0);
}
/* }}} SapiResponse::getContent */

/* {{{ proto SapiResponseInterface SapiResponse::setContent(mixed $content) */
PHP_METHOD(SapiResponse, setContent)
{
    zval *_this_zval = getThis();
    zval *content;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(content)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property(SapiResponse_ce_ptr, _this_zval, ZEND_STRL("content"), content);

    RETURN_ZVAL(_this_zval, 1, 0);
}
/* }}} SapiResponse::setContent */

/* {{{ proto SapiResponseInterface SapiResponse::addHeaderCallback(callable $callback) */
PHP_METHOD(SapiResponse, addHeaderCallback)
{
    zval *callback_func;
    zval *_this_zval = getThis();
    zval *prop_ptr;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(callback_func)
    ZEND_PARSE_PARAMETERS_END();

    if( Z_TYPE_P(callback_func) == IS_NULL || !zend_is_callable(callback_func, 0, NULL) ) {
        RETURN_FALSE;
    }

    // Read property pointer
    if( !Z_OBJ_HT_P(_this_zval)->get_property_ptr_ptr ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "SapiResponse::addHeaderCallback requires get_property_ptr_ptr");
        return;
    }

    do {
#if PHP_MAJOR_VERSION >= 8
        zend_string *member = zend_string_init(ZEND_STRL("callbacks"), 0);
        prop_ptr = Z_OBJ_HT_P(_this_zval)->get_property_ptr_ptr(Z_OBJ_P(_this_zval), member, BP_VAR_RW, NULL);
        zend_string_release(member);
#else
        zval member = {0};
        ZVAL_STRING(&member, "callbacks");
        prop_ptr = Z_OBJ_HT_P(_this_zval)->get_property_ptr_ptr(_this_zval, &member, BP_VAR_RW, NULL);
        zval_ptr_dtor(&member);
#endif
    } while (0);

    if( !prop_ptr || Z_TYPE_P(prop_ptr) != IS_ARRAY ) {
        convert_to_array(prop_ptr);
    }

    // Append callback
    zval tmp = {0};
    ZVAL_ZVAL(&tmp, callback_func, 1, 0);
    add_next_index_zval(prop_ptr, &tmp);

    RETURN_ZVAL(_this_zval, 1, 0);
}
/* }}} SapiResponse::setHeaderCallbacks */

/* {{{ proto SapiResponseInterface SapiResponse::setHeaderCallbacks(array $callbacks) */
PHP_METHOD(SapiResponse, setHeaderCallbacks)
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
    zend_update_property(SapiResponse_ce_ptr, _this_zval, ZEND_STRL("callbacks"), &arr);

    // Forward each item to addHeaderCallback
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(callbacks), callback) {
#if PHP_MAJOR_VERSION >= 8
        zend_call_method_with_1_params(Z_OBJ_P(_this_zval), NULL, NULL, "addHeaderCallback", NULL, callback);
#else
        zend_call_method_with_1_params(_this_zval, NULL, NULL, "addHeaderCallback", NULL, callback);
#endif
    } ZEND_HASH_FOREACH_END();

    RETURN_ZVAL(_this_zval, 1, 0);
}
/* }}} SapiResponse::setHeaderCallbacks */

/* {{{ proto callable[] SapiResponse::getHeaderCallbacks() */
static zval *sapi_response_get_header_callbacks(zval *response)
{
    return zend_read_property(SapiResponse_ce_ptr, response, ZEND_STRL("callbacks"), 0, NULL);
}

PHP_METHOD(SapiResponse, getHeaderCallbacks)
{
    zval *_this_zval = getThis();

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    RETVAL_ZVAL(sapi_response_get_header_callbacks(_this_zval), 1, 0);
}
/* }}} SapiResponse::getHeaderCallback */

/* {{{ SapiResponse methods */
static zend_function_entry SapiResponse_methods[] = {
    PHP_ME(SapiResponse, setVersion, SapiResponseInterface_setVersion_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(SapiResponse, getVersion, SapiResponseInterface_getVersion_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(SapiResponse, setCode, SapiResponseInterface_setCode_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(SapiResponse, getCode, SapiResponseInterface_getCode_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(SapiResponse, setHeader, SapiResponseInterface_addHeader_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(SapiResponse, addHeader, SapiResponseInterface_setHeader_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(SapiResponse, unsetHeader, SapiResponseInterface_unsetHeader_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(SapiResponse, getHeader, SapiResponseInterface_getHeader_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(SapiResponse, hasHeader, SapiResponseInterface_hasHeader_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(SapiResponse, unsetHeaders, SapiResponseInterface_unsetHeaders_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(SapiResponse, getHeaders, SapiResponseInterface_getHeaders_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(SapiResponse, setCookie, SapiResponseInterface_setCookie_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(SapiResponse, setRawCookie, SapiResponseInterface_setCookie_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(SapiResponse, unsetCookie, SapiResponseInterface_unsetCookie_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(SapiResponse, unsetCookies, SapiResponseInterface_unsetCookies_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(SapiResponse, getCookie, SapiResponseInterface_getCookie_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(SapiResponse, hasCookie, SapiResponseInterface_hasCookie_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(SapiResponse, getCookies, SapiResponseInterface_getCookies_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(SapiResponse, setContent, SapiResponseInterface_setContent_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(SapiResponse, getContent, SapiResponseInterface_getContent_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(SapiResponse, setHeaderCallbacks, SapiResponseInterface_setHeaderCallbacks_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(SapiResponse, addHeaderCallback, SapiResponseInterface_addHeaderCallback_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(SapiResponse, getHeaderCallbacks, SapiResponseInterface_getHeaderCallbacks_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_FE_END
};
/* }}} SapiResponse methods */

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(sapiresponse)
{
    zend_class_entry SapiResponse_ce;

    INIT_CLASS_ENTRY(SapiResponse_ce, "SapiResponse", SapiResponse_methods);
    SapiResponse_ce_ptr = zend_register_internal_class(&SapiResponse_ce);
    zend_class_implements(SapiResponse_ce_ptr, 1, SapiResponseInterface_ce_ptr);

    zend_declare_property_null(SapiResponse_ce_ptr, ZEND_STRL("version"), ZEND_ACC_PRIVATE);
    zend_declare_property_null(SapiResponse_ce_ptr, ZEND_STRL("code"), ZEND_ACC_PRIVATE);
    zend_declare_property_null(SapiResponse_ce_ptr, ZEND_STRL("headers"), ZEND_ACC_PRIVATE);
    zend_declare_property_null(SapiResponse_ce_ptr, ZEND_STRL("cookies"), ZEND_ACC_PRIVATE);
    zend_declare_property_null(SapiResponse_ce_ptr, ZEND_STRL("content"), ZEND_ACC_PRIVATE);
    zend_declare_property_null(SapiResponse_ce_ptr, ZEND_STRL("callbacks"), ZEND_ACC_PRIVATE);

    return SUCCESS;
}

/* SapiResponseSender *********************************************************** */

/* {{{ Argument Info */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(SapiResponseSender_send_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, response, SapiResponseInterface, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(SapiResponseSender_runHeaderCallbacks_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, response, SapiResponseInterface, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(SapiResponseSender_sendStatus_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, response, SapiResponseInterface, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(SapiResponseSender_sendHeaders_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, response, SapiResponseInterface, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(SapiResponseSender_sendCookies_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, response, SapiResponseInterface, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(SapiResponseSender_sendContent_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, response, SapiResponseInterface, 0)
ZEND_END_ARG_INFO()

/* }}} Argument Info */

/* {{{ proto void SapiResponseSender::runHeaderCallbacks() */
static void sapi_response_sender_run_header_callbacks(zval *response)
{
    zval callbacks = {0};
    zval *callback;

#if PHP_MAJOR_VERSION >= 8
    zend_call_method_with_0_params(Z_OBJ_P(response), Z_OBJCE_P(response), NULL, "getheadercallbacks", &callbacks);
#else
    zend_call_method_with_0_params(response, Z_OBJCE_P(response), NULL, "getheadercallbacks", &callbacks);
#endif

    if( Z_TYPE(callbacks) != IS_ARRAY ) {
        zval_ptr_dtor(&callbacks);
        return;
    }

    ZEND_HASH_FOREACH_VAL(Z_ARRVAL(callbacks), callback) {

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

    zval_ptr_dtor(&callbacks);
}

PHP_METHOD(SapiResponseSender, runHeaderCallbacks)
{
    zval *response;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(response, SapiResponseInterface_ce_ptr)
    ZEND_PARSE_PARAMETERS_END();

    sapi_response_sender_run_header_callbacks(response);
}
/* }}} SapiResponseSender::runHeaderCallbacks */

/* {{{ proto void SapiResponseSender::sendStatus() */
static void sapi_response_sender_send_status(zval *response)
{
    sapi_header_line ctr = {0};
    zval code = {0};
    zval version = {0};
    smart_str buf = {0};

#if PHP_MAJOR_VERSION >= 8
    zend_call_method_with_0_params(Z_OBJ_P(response), Z_OBJCE_P(response), NULL, "getcode", &code);
    zend_call_method_with_0_params(Z_OBJ_P(response), Z_OBJCE_P(response), NULL, "getversion", &version);
#else
    zend_call_method_with_0_params(response, Z_OBJCE_P(response), NULL, "getcode", &code);
    zend_call_method_with_0_params(response, Z_OBJCE_P(response), NULL, "getversion", &version);
#endif

    // Make code
    if( Z_TYPE(code) == IS_LONG ) {
        ctr.response_code = zval_get_long(&code);
    } else {
        ctr.response_code = 200;
    }

    // Make status
    smart_str_appendl_ex(&buf, ZEND_STRL("HTTP/"), 0);

    if (Z_TYPE(version) == IS_NULL || Z_TYPE(version) == IS_UNDEF) {
        smart_str_appendl_ex(&buf, ZEND_STRL("1.1"), 0);
    } else {
        convert_to_string(&version);
        smart_str_append_ex(&buf, Z_STR(version), 0);
    }

    smart_str_appendc_ex(&buf, ' ', 0);
    smart_str_append_long_ex(&buf, ctr.response_code, 0);
    smart_str_0(&buf);

    ctr.line = ZSTR_VAL(buf.s);
    ctr.line_len = ZSTR_LEN(buf.s);

    sapi_header_op(SAPI_HEADER_REPLACE, &ctr);

    smart_str_free(&buf);

    zval_ptr_dtor(&code);
    zval_ptr_dtor(&version);
}

PHP_METHOD(SapiResponseSender, sendStatus)
{
    zval *response;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(response, SapiResponseInterface_ce_ptr)
    ZEND_PARSE_PARAMETERS_END();

    sapi_response_sender_send_status(response);
}
/* }}} SapiResponseSender::sendStatus */

/* {{{ proto void SapiResponseSender::sendHeaders() */
static inline void sapi_response_sender_send_header(zend_string *header, zval *value)
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

static void sapi_response_sender_send_headers(zval *response)
{
    zval headers = {0};
    zend_ulong index;
    zend_string *label;
    zval *value;

#if PHP_MAJOR_VERSION >= 8
    zend_call_method_with_0_params(Z_OBJ_P(response), Z_OBJCE_P(response), NULL, "getheaders", &headers);
#else
    zend_call_method_with_0_params(response, Z_OBJCE_P(response), NULL, "getheaders", &headers);
#endif

    if( Z_TYPE(headers) == IS_ARRAY ) {
        ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(headers), index, label, value) {
            if( label ) {
                sapi_response_sender_send_header(label, value);
            }
        } ZEND_HASH_FOREACH_END();
    }

    zval_ptr_dtor(&headers);
}

PHP_METHOD(SapiResponseSender, sendHeaders)
{
    zval *response;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(response, SapiResponseInterface_ce_ptr)
    ZEND_PARSE_PARAMETERS_END();

    sapi_response_sender_send_headers(response);
}
/* }}} SapiResponseSender::sendHeaders */

/* {{{ proto void SapiResponseSender::sendCookies() */
static inline void sapi_response_sender_send_cookie(zend_string *name, zval *arr)
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

static void sapi_response_sender_send_cookies(zval *response)
{
    zval cookies = {0};
    zend_string *key;
    zend_ulong index;
    zval *val;

#if PHP_MAJOR_VERSION >= 8
    zend_call_method_with_0_params(Z_OBJ_P(response), Z_OBJCE_P(response), NULL, "getcookies", &cookies);
#else
    zend_call_method_with_0_params(response, Z_OBJCE_P(response), NULL, "getcookies", &cookies);
#endif

    if( Z_TYPE(cookies) == IS_ARRAY ) {
        ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(cookies), index, key, val) {
            if( key && Z_TYPE_P(val) == IS_ARRAY ) {
                sapi_response_sender_send_cookie(key, val);
            }
        } ZEND_HASH_FOREACH_END();
    }

    zval_ptr_dtor(&cookies);
}

PHP_METHOD(SapiResponseSender, sendCookies)
{
    zval *response;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(response, SapiResponseInterface_ce_ptr)
    ZEND_PARSE_PARAMETERS_END();

    sapi_response_sender_send_cookies(response);
}
/* }}} SapiResponseSender::sendCookies */

/* {{{ proto void SapiResponseSender::sendContent() */
static inline void sapi_response_sender_send_iterable(zval *content) {
    zend_object_iterator *iter;
    zend_class_entry *ce = Z_OBJCE_P(content);
    zval *val;
    zend_string *val_str;

    iter = ce->get_iterator(ce, content, 0);

    if (EG(exception)) {
        goto done;
    }

    if (iter->funcs->rewind) {
        iter->funcs->rewind(iter);
        if (EG(exception)) {
            goto done;
        }
    }

    while (iter->funcs->valid(iter) == SUCCESS) {
        if (EG(exception)) {
            goto done;
        }

        val = iter->funcs->get_current_data(iter);
        if (EG(exception)) {
            goto done;
        }

        val_str = zval_get_string(val);
        php_output_write(ZSTR_VAL(val_str), ZSTR_LEN(val_str));
        zend_string_release(val_str);
        zval_ptr_dtor(val);

        iter->funcs->move_forward(iter);
        if (EG(exception)) {
            goto done;
        }
    }

done:
    OBJ_RELEASE(&iter->std);
}

static void sapi_response_sender_send_content(zval *response)
{
    zval *content;
    zend_string *content_str;
    php_stream *stream;
    zval func_name = {0};
    zval rv = {0}, rv2 = {0};
    zval params[1] = {0};

    // Call getContent
#if PHP_MAJOR_VERSION >= 8
    zend_call_method_with_0_params(Z_OBJ_P(response), Z_OBJCE_P(response), NULL, "getcontent", &rv2);
#else
    zend_call_method_with_0_params(response, Z_OBJCE_P(response), NULL, "getcontent", &rv2);
#endif
    content = &rv2;

    // Call again if the value was callable
    if( Z_TYPE_P(content) == IS_OBJECT && zend_is_callable(content, 0, NULL) ) {
        ZVAL_STRING(&func_name, "__invoke");
        ZVAL_ZVAL(&params[0], response, 1, 0);
        call_user_function(&Z_OBJCE_P(content)->function_table, content, &func_name, &rv, 1, params);
        zval_ptr_dtor(&func_name);
        zval_ptr_dtor(&params[0]);
        content = &rv;
        zval_ptr_dtor(&rv2);
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
            // ported from php_stream_from_res
            stream = zend_fetch_resource2(Z_RES_P(content), "stream", php_file_le_stream(), php_file_le_pstream());
            if (stream) {
                php_stream_seek(stream, 0, SEEK_SET);
                php_stream_passthru(stream);
            }
            break;

        case IS_STRING:
            php_output_write(Z_STRVAL_P(content), Z_STRLEN_P(content));
            break;

        case IS_OBJECT:
            if (instanceof_function(Z_OBJCE_P(content), zend_ce_traversable)) {
                sapi_response_sender_send_iterable(content);
                break;
            }
            // !fallthrough to default!

        default:
            content_str = zval_get_string(content);
            php_output_write(ZSTR_VAL(content_str), ZSTR_LEN(content_str));
            zend_string_release(content_str);
            break;
    }

    zval_ptr_dtor(content);
}

PHP_METHOD(SapiResponseSender, sendContent)
{
    zval *response;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(response, SapiResponseInterface_ce_ptr)
    ZEND_PARSE_PARAMETERS_END();

    sapi_response_sender_send_content(response);

}
/* }}} SapiResponseSender::sendContent */

/* {{{ proto void SapiResponseSender::send() */
PHP_METHOD(SapiResponseSender, send)
{
    zval *response;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(response, SapiResponseInterface_ce_ptr)
    ZEND_PARSE_PARAMETERS_END();

    sapi_response_sender_run_header_callbacks(response);
    sapi_response_sender_send_status(response);
    sapi_response_sender_send_headers(response);
    sapi_response_sender_send_cookies(response);
    sapi_response_sender_send_content(response);
}
/* }}} SapiResponseSender::send */

/* {{{ SapiResponseSender methods */
static zend_function_entry SapiResponseSender_methods[] = {
    PHP_ME(SapiResponseSender, send, SapiResponseSender_send_args, ZEND_ACC_PUBLIC)
    PHP_ME(SapiResponseSender, runHeaderCallbacks, SapiResponseSender_runHeaderCallbacks_args, ZEND_ACC_PUBLIC)
    PHP_ME(SapiResponseSender, sendStatus, SapiResponseSender_sendStatus_args, ZEND_ACC_PUBLIC)
    PHP_ME(SapiResponseSender, sendHeaders, SapiResponseSender_sendHeaders_args, ZEND_ACC_PUBLIC)
    PHP_ME(SapiResponseSender, sendCookies, SapiResponseSender_sendCookies_args, ZEND_ACC_PUBLIC)
    PHP_ME(SapiResponseSender, sendContent, SapiResponseSender_sendContent_args, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
/* }}} SapiResponseSender methods */

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(sapiresponsesender)
{
    zend_class_entry SapiResponseSender_ce;

    INIT_CLASS_ENTRY(SapiResponseSender_ce, "SapiResponseSender", SapiResponseSender_methods);
    SapiResponseSender_ce_ptr = zend_register_internal_class(&SapiResponseSender_ce);

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
