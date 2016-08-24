
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "main/php.h"
#include "main/php_ini.h"
#include "ext/standard/info.h"
#include "ext/spl/spl_exceptions.h"
#include "Zend/zend_API.h"

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
PHP_METHOD(PhpRequest, __construct)
{
    zval * _this_zval = getThis();
    zend_string * method = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 1)
            Z_PARAM_OPTIONAL
            Z_PARAM_STR(method)
    ZEND_PARSE_PARAMETERS_END();

    struct php_request_obj * intern = Z_REQUEST_P(_this_zval);



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

    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("method"), ZEND_ACC_PUBLIC TSRMLS_CC);

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
