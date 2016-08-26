
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "main/php.h"
#include "main/php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "ext/standard/url.h"
#include "ext/spl/spl_exceptions.h"
#include "Zend/zend_API.h"
#include "Zend/zend_portability.h"
#include "Zend/zend_smart_str.h"

#include "php_request.h"

zend_class_entry * PhpRequest_ce_ptr;
static zend_object_handlers PhpRequest_obj_handlers;
static HashTable PhpRequest_prop_handlers;

struct php_request_obj {
    zend_bool locked;
    zend_object std;
};

/* {{{ Argument Info */
ZEND_BEGIN_ARG_INFO_EX(PhpRequest_construct_args, 0, 0, 0)
    ZEND_ARG_INFO(0, method)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(PhpRequest_parseAccepts_args, 0, 0, 0)
    ZEND_ARG_INFO(0, header)
ZEND_END_ARG_INFO()
/* }}} Argument Info */

/* {{{ Z_REQUEST_P */
static inline struct php_request_obj * php_request_fetch_object(zend_object *obj) {
    return (struct php_request_obj *)((char*)(obj) - XtOffsetOf(struct php_request_obj, std));
}
#define Z_REQUEST_P(zv) php_request_fetch_object(Z_OBJ_P((zv)))
/* }}} */

/* {{{ php_request_obj_create */
static zend_object * php_request_obj_create(zend_class_entry * ce)
{
    struct php_request_obj *obj;

    obj = ecalloc(1, sizeof(*obj) + zend_object_properties_size(ce));
    zend_object_std_init(&obj->std, ce);
    object_properties_init(&obj->std, ce);
    obj->std.handlers = &PhpRequest_obj_handlers;

    obj->locked = 0;

    return &obj->std;
}
/* }}} */

/* {{{ php_request_object_has_property */
static int php_request_object_has_property(zval *object, zval *member, int has_set_exists, void **cache_slot)
{
    if( !std_object_handlers.has_property(object, member, has_set_exists, cache_slot) ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "PhpRequest::$%s does not exist.", Z_STRVAL_P(member));
        return 0;
    } else {
        return 1;
    }
}
/* }}} */

/* {{{ php_request_object_read_property */
static zval* php_request_object_read_property(zval *object, zval *member, int type, void **cache_slot, zval *rv)
{
    if( !std_object_handlers.has_property(object, member, 2, cache_slot) ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "PhpRequest::$%s does not exist.", Z_STRVAL_P(member));
        return rv;
    } else {
        return std_object_handlers.read_property(object, member, type, cache_slot, rv);
    }
}
/* }}} */

/* {{{ php_request_object_write_property */
static void php_request_object_write_property(zval *object, zval *member, zval *value, void **cache_slot)
{
    struct php_request_obj * intern = Z_REQUEST_P(object);
    if( intern->locked ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "PhpRequest is read-only.");
    } else {
        std_object_handlers.write_property(object, member, value, cache_slot);
    }
}
/* }}} */

/* {{{ php_request_object_unset_property */
static void php_request_object_unset_property(zval *object, zval *member, void **cache_slot)
{
    struct php_request_obj * intern = Z_REQUEST_P(object);
    if( intern->locked ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "PhpRequest is read-only.");
    } else {
        std_object_handlers.unset_property(object, member, cache_slot);
    }
}
/* }}} */


/* {{{ proto PhpRequest::__construct([string $method]) */
static inline void copy_global(zval* obj, const char* key, size_t key_len, const char* sg, size_t sg_len)
{
    zval * tmp = zend_hash_str_find(&EG(symbol_table), sg, sg_len);
    if( tmp ) {
        zend_update_property(Z_CE_P(obj), obj, key, key_len, tmp);
    }
}
#define copy_global_lit(obj, glob, key) copy_global(obj, ZEND_STRL(glob), ZEND_STRL(key))

static inline const unsigned char * extract_port(const unsigned char * host, size_t len)
{
    const unsigned char * right = host + len - 1;
    const unsigned char * left = len > 6 ? right - 6 : host;
    const unsigned char * pos = right;
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

static inline void set_url(zval *object, zval *server, zend_bool secure)
{
    zval rv = {0};
    zval *tmp;
    zend_bool is_secure = 0;
    zend_string * host = NULL;
    zend_long port = 0;
    zend_string * uri = NULL;
    smart_str buf = {0};
    php_url * url;
    zval arr = {0};

    if( !server || Z_TYPE_P(server) != IS_ARRAY ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "Could not determine host for PhpRequest.");
        return;
    }

    // Get scheme
    tmp = zend_read_property(Z_CE_P(object), object, ZEND_STRL("secure"), 0, &rv);
    if( tmp && Z_TYPE_P(tmp) == IS_TRUE ) {
        is_secure = 1;
    }

    // Get host
    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_HOST"))) &&
            Z_TYPE_P(tmp) == IS_STRING ) {
        host = Z_STR_P(tmp);
    } else if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("SERVER_NAME"))) &&
            Z_TYPE_P(tmp) == IS_STRING ) {
        host = Z_STR_P(tmp);
    } else {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "Could not determine host for PhpRequest.");
        return;
    }

    // Get port
    if( NULL != extract_port(ZSTR_VAL(host), ZSTR_LEN(host)) ) {
        // no need to extract
    } else if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("SERVER_PORT"))) ) {
        convert_to_long(tmp);
        port = Z_LVAL_P(tmp);
    }

    // Get uri
    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("REQUEST_URI"))) &&
        Z_TYPE_P(tmp) == IS_STRING ) {
        uri = Z_STR_P(tmp);
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

    // Re-parse URL
    url = php_url_parse_ex(ZSTR_VAL(buf.s), ZSTR_LEN(buf.s));
    smart_str_free(&buf);
    if( !url ) {
        return;
    }

    // Form array
    array_init(&arr);
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

    convert_to_object(&arr);

    zend_update_property(Z_CE_P(object), object, ZEND_STRL("url"), &arr);
}

static inline void set_accept_by_name(zval *object, zval *server, const char *src, size_t src_length, const char *dest, size_t dest_length)
{
    zval val;
    zval *tmp;

    array_init(&val);
    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), src, src_length)) ) {
        php_request_parse_accepts(&val, Z_STRVAL_P(tmp), Z_STRLEN_P(tmp));
    }
    zend_update_property(Z_CE_P(object), object, dest, dest_length, &val);
}

PHP_METHOD(PhpRequest, __construct)
{
    zval * _this_zval = getThis();
    zend_string * method = NULL;
    zval *server;
    zval rv;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(method)
    ZEND_PARSE_PARAMETERS_END();

    struct php_request_obj * intern = Z_REQUEST_P(_this_zval);

    // Copy superglobals
    copy_global_lit(_this_zval, "env", "_ENV");
    copy_global_lit(_this_zval, "server", "_SERVER");

    copy_global_lit(_this_zval, "cookie", "_COOKIE");
    copy_global_lit(_this_zval, "files", "_FILES");
    copy_global_lit(_this_zval, "get", "_GET");
    copy_global_lit(_this_zval, "post", "_POST");

    // Read back server property
    server = zend_read_property(Z_CE_P(_this_zval), _this_zval, ZEND_STRL("server"), 0, &rv);

    // method
    zval zmethod = {0};
    ZVAL_STRING(&zmethod, "");
    zend_bool xhr = php_request_detect_method(&zmethod, server, method);
    zend_update_property(Z_CE_P(_this_zval), _this_zval, ZEND_STRL("method"), &zmethod);
    zend_update_property_bool(Z_CE_P(_this_zval), _this_zval, ZEND_STRL("xhr"), xhr);

    // Internal setters that require server
    if( server && Z_TYPE_P(server) == IS_ARRAY ) {
        // secure
        zend_bool secure = php_request_is_secure(server);
        zend_update_property_bool(Z_CE_P(_this_zval), _this_zval, ZEND_STRL("secure"), secure);

        // headers
        zval headers = {0};
        php_request_normalize_headers(&headers, server);
        zend_update_property(Z_CE_P(_this_zval), _this_zval, ZEND_STRL("headers"), &headers);

        // url
        set_url(_this_zval, server, secure);

        // accepts
        set_accept_by_name(_this_zval, server, ZEND_STRL("HTTP_ACCEPT"), ZEND_STRL("acceptMedia"));
        set_accept_by_name(_this_zval, server, ZEND_STRL("HTTP_ACCEPT_CHARSET"), ZEND_STRL("acceptCharset"));
        set_accept_by_name(_this_zval, server, ZEND_STRL("HTTP_ACCEPT_ENCODING"), ZEND_STRL("acceptEncoding"));
        //set_accept_by_name(_this_zval, server, ZEND_STRL("HTTP_ACCEPT_LANGUAGE"), ZEND_STRL("acceptLanguage"));
    }

    // Lock the object
    intern->locked = 1;
}
/* }}} PhpRequest::__construct */

/* {{{ proto PhpRequest::__construct([string $method]) */
PHP_METHOD(PhpRequest, parseAccepts)
{
    zend_string * header;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(header)
    ZEND_PARSE_PARAMETERS_END();

    php_request_parse_accepts(return_value, ZSTR_VAL(header), ZSTR_LEN(header));

}
/* }}} PhpRequest::__construct */

/* {{{ PhpRequest methods */
static zend_function_entry PhpRequest_methods[] = {
    PHP_ME(PhpRequest, __construct, PhpRequest_construct_args, ZEND_ACC_PUBLIC)
    PHP_ME(PhpRequest, parseAccepts, PhpRequest_parseAccepts_args, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_FE_END
};
/* }}} PhpRequest methods */

/* {{{ PHP_MINIT_FUNCTION */
static PHP_MINIT_FUNCTION(request)
{
    zend_class_entry ce;

    memcpy(&PhpRequest_obj_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    PhpRequest_obj_handlers.offset = XtOffsetOf(struct php_request_obj, std);
    PhpRequest_obj_handlers.has_property = php_request_object_has_property;
    PhpRequest_obj_handlers.read_property = php_request_object_read_property;
    PhpRequest_obj_handlers.write_property = php_request_object_write_property;
    PhpRequest_obj_handlers.unset_property = php_request_object_unset_property;

    INIT_CLASS_ENTRY(ce, "PhpRequest", PhpRequest_methods);
    PhpRequest_ce_ptr = zend_register_internal_class(&ce);
    PhpRequest_ce_ptr->create_object = php_request_obj_create;

    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("acceptCharset"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("acceptEncoding"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("acceptLanguage"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("acceptMedia"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("cookie"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("env"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("files"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("get"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("headers"), ZEND_ACC_PUBLIC);
    zend_declare_property_string(PhpRequest_ce_ptr, ZEND_STRL("method"), "", ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("post"), ZEND_ACC_PUBLIC);
    zend_declare_property_bool(PhpRequest_ce_ptr, ZEND_STRL("secure"), 0, ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("server"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("url"), ZEND_ACC_PUBLIC);
    zend_declare_property_bool(PhpRequest_ce_ptr, ZEND_STRL("xhr"), 0, ZEND_ACC_PUBLIC);

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION */
static PHP_MINFO_FUNCTION(request)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "Version", PHP_REQUEST_VERSION);
    php_info_print_table_end();
}
/* }}} */

zend_module_entry request_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_REQUEST_NAME,                   /* Name */
    NULL,                               /* Functions */
    PHP_MINIT(request),                 /* MINIT */
    NULL,                               /* MSHUTDOWN */
    NULL,                               /* RINIT */
    NULL,                               /* RSHUTDOWN */
    PHP_MINFO(request),                 /* MINFO */
    PHP_REQUEST_VERSION,                /* Version */
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_REQUEST
ZEND_GET_MODULE(request)      // Common for all PHP extensions which are build as shared modules
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: et sw=4 ts=4
 */