
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "main/php.h"
#include "main/php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "ext/spl/spl_exceptions.h"
#include "Zend/zend_API.h"
#include "Zend/zend_portability.h"

#include "php_request.h"

zend_class_entry * PhpRequest_ce_ptr;
static zend_object_handlers PhpRequest_obj_handlers;
static HashTable PhpRequest_prop_handlers;

struct php_request_obj {
    zend_object std;
    zend_bool locked;
};

/* {{{ Argument Info */
ZEND_BEGIN_ARG_INFO_EX(PhpRequest_construct_args, ZEND_SEND_BY_VAL, 0, 0)
                ZEND_ARG_INFO(0, method)
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
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "PhpRequest::%s does not exist.", Z_STRVAL_P(member));
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
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "PhpRequest::%s does not exist.", Z_STRVAL_P(member));
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

static inline void set_method(zval* object, zend_string * method)
{
    zval rv;
    zend_string* tmp;
    zval* server;
    zval* val;

    // force the method?
    if( method && ZSTR_LEN(method) > 0 ) {
        tmp = zend_string_dup(method, 0);
        php_strtoupper(ZSTR_VAL(tmp), ZSTR_LEN(tmp));
        zend_update_property_str(Z_CE_P(object), object, ZEND_STRL("method"), method);
        zend_string_release(tmp);
        return;
    }

    // get server
    server = zend_read_property(Z_CE_P(object), object, ZEND_STRL("server"), 0, &rv);
    if( !server || Z_TYPE_P(server) != IS_ARRAY ) {
        return;
    }

    // determine method from request
    val = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("REQUEST_METHOD"));
    if( !val || Z_TYPE_P(val) != IS_STRING ) {
        return;
    }
    method = Z_STR_P(val);

    // XmlHttpRequest method override?
    if( zend_string_equals_literal_ci(method, "POST") ) {
        val = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_X_HTTP_METHOD_OVERRIDE"));
        if( val && Z_TYPE_P(val) == IS_STRING ) {
            method = Z_STR_P(val);
            zend_update_property_bool(Z_CE_P(object), object, ZEND_STRL("xhr"), 1);
        }
    }

    tmp = zend_string_dup(method, 0);
    php_strtoupper(ZSTR_VAL(tmp), ZSTR_LEN(tmp));
    zend_update_property_str(Z_CE_P(object), object, ZEND_STRL("method"), method);
    zend_string_release(tmp);
}

PHP_METHOD(PhpRequest, __construct)
{
    zval * _this_zval = getThis();
    zend_string * method = NULL;

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

    set_method(_this_zval, method);

    // Lock the object
    intern->locked = 1;
}
/* }}} PhpRequest::__construct */

/* {{{ PhpRequest methods */
static zend_function_entry PhpRequest_methods[] = {
        PHP_ME(PhpRequest, __construct, PhpRequest_construct_args, ZEND_ACC_PUBLIC)
        PHP_FE_END
};
/* }}} PhpRequest methods */

/* {{{ PHP_MINIT_FUNCTION */
static PHP_MINIT_FUNCTION(request)
{
    zend_class_entry ce;

    memcpy(&PhpRequest_obj_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    PhpRequest_obj_handlers.has_property = php_request_object_has_property;
    PhpRequest_obj_handlers.read_property = php_request_object_read_property;
    PhpRequest_obj_handlers.write_property = php_request_object_write_property;
    PhpRequest_obj_handlers.unset_property = php_request_object_unset_property;

    INIT_CLASS_ENTRY(ce, "PhpRequest", PhpRequest_methods);
    PhpRequest_ce_ptr = zend_register_internal_class(&ce);
    PhpRequest_ce_ptr->create_object = php_request_obj_create;

    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("acceptCharset"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("acceptEncoding"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("acceptLanguage"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("acceptMedia"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("cookie"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("env"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("files"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("get"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("headers"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_string(PhpRequest_ce_ptr, ZEND_STRL("method"), "", ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("post"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_bool(PhpRequest_ce_ptr, ZEND_STRL("secure"), 0, ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("server"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("url"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_bool(PhpRequest_ce_ptr, ZEND_STRL("xhr"), 0, ZEND_ACC_PUBLIC TSRMLS_CC);

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
