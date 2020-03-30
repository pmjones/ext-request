
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "main/php.h"

#include "php_request.h"
#include "request_utils.h"



PHP_REQUEST_API zend_class_entry *SapiUpload_ce_ptr;

static zend_object_handlers SapiUpload_obj_handlers;
static HashTable SapiUpload_prop_handlers;

/* {{{ request_sapiupload_obj_create */
static zend_object *request_sapiupload_obj_create(zend_class_entry *ce)
{
    zend_object *obj;

    obj = ecalloc(1, sizeof(*obj) + zend_object_properties_size(ce));
    zend_object_std_init(obj, ce);
    object_properties_init(obj, ce);
    obj->handlers = &SapiUpload_obj_handlers;

    return obj;
}
/* }}} */

/* {{{ request_sapiupload_clone_obj */
#if PHP_MAJOR_VERSION >= 8
static zend_object *request_sapiupload_clone_obj(zend_object *zobject)
{
    zend_object * new_obj = std_object_handlers.clone_obj(zobject);
    new_obj->handlers = &SapiUpload_obj_handlers;
    return new_obj;
}
#else
static zend_object *request_sapiupload_clone_obj(zval *zobject)
{
    zend_object * new_obj = std_object_handlers.clone_obj(zobject);
    new_obj->handlers = &SapiUpload_obj_handlers;
    return new_obj;
}
#endif
/* }}} */

/* {{{ request_sapiupload_object_default_write_property */
#if PHP_MAJOR_VERSION >= 8
static zval *request_sapiupload_object_default_write_property(zend_object *object, zend_string *member, zval *value, void **cache_slot)
{
    return request_readonly_write_property_handler(SapiUpload_ce_ptr, object, member, value, cache_slot);
}
#elif PHP_MAJOR_VERSION >= 7 && PHP_MINOR_VERSION >= 4
static zval *request_sapiupload_object_default_write_property(zval *object, zval *member, zval *value, void **cache_slot)
{
    return request_readonly_write_property_handler(SapiUpload_ce_ptr, object, member, value, cache_slot);
}
#else
static void request_sapiupload_object_default_write_property(zval *object, zval *member, zval *value, void **cache_slot)
{
    request_readonly_write_property_handler(SapiUpload_ce_ptr, object, member, value, cache_slot);
}
#endif
/* }}} */

/* {{{ request_sapiupload_object_default_unset_property */
#if PHP_MAJOR_VERSION >= 8
static void request_sapiupload_object_default_unset_property(zend_object *object, zend_string *member, void **cache_slot)
{
    request_readonly_unset_property_handler(SapiUpload_ce_ptr, object, member, cache_slot);
}
#else
static void request_sapiupload_object_default_unset_property(zval *object, zval *member, void **cache_slot)
{
    request_readonly_unset_property_handler(SapiUpload_ce_ptr, object, member, cache_slot);
}
#endif
/* }}} */

/* {{{ request_sapiupload_object_has_property */
#if PHP_MAJOR_VERSION >= 8
static int request_sapiupload_object_has_property(zend_object *object, zend_string *member, int has_set_exists, void **cache_slot)
{
    return request_has_property_dispatcher(&SapiUpload_prop_handlers, object, member, has_set_exists, cache_slot);
}
#else
static int request_sapiupload_object_has_property(zval *object, zval *member, int has_set_exists, void **cache_slot)
{
    return request_has_property_dispatcher(&SapiUpload_prop_handlers, object, member, has_set_exists, cache_slot);
}
#endif
/* }}} */

/* {{{ request_sapiupload_object_read_property */
#if PHP_MAJOR_VERSION >= 8
static zval *request_sapiupload_object_read_property(zend_object *object, zend_string *member, int type, void **cache_slot, zval *rv)
{
    return request_read_property_dispatcher(&SapiUpload_prop_handlers, object, member, type, cache_slot, rv);
}
#else
static zval *request_sapiupload_object_read_property(zval *object, zval *member, int type, void **cache_slot, zval *rv)
{
    return request_read_property_dispatcher(&SapiUpload_prop_handlers, object, member, type, cache_slot, rv);
}
#endif
/* }}} */

/* {{{ request_sapiupload_object_write_property */
#if PHP_MAJOR_VERSION >= 8
static zval *request_sapiupload_object_write_property(zend_object *object, zend_string *member, zval *value, void **cache_slot)
{
    return request_write_property_dispatcher(&SapiUpload_prop_handlers, object, member, value, cache_slot);
}
#elif PHP_MAJOR_VERSION >= 7 && PHP_MINOR_VERSION >= 4
static zval *request_sapiupload_object_write_property(zval *object, zval *member, zval *value, void **cache_slot)
{
    return request_write_property_dispatcher(&SapiUpload_prop_handlers, object, member, value, cache_slot);
}
#else
static void request_sapiupload_object_write_property(zval *object, zval *member, zval *value, void **cache_slot)
{
    request_write_property_dispatcher(&SapiUpload_prop_handlers, object, member, value, cache_slot);
}
#endif
/* }}} */

/* {{{ request_sapiupload_object_unset_property */
#if PHP_MAJOR_VERSION >= 8
static void request_sapiupload_object_unset_property(zend_object *object, zend_string *member, void **cache_slot)
{
    return request_unset_property_dispatcher(&SapiUpload_prop_handlers, object, member, cache_slot);
}
#else
static void request_sapiupload_object_unset_property(zval *object, zval *member, void **cache_slot)
{
    return request_unset_property_dispatcher(&SapiUpload_prop_handlers, object, member, cache_slot);
}
#endif
/* }}} */

/* {{{ request_sapiupload_object_get_property_ptr_ptr */
#if PHP_MAJOR_VERSION >= 8
static zval *request_sapiupload_object_get_property_ptr_ptr(zend_object *object, zend_string *name, int type, void **cache_slot)
{
    return NULL;
}
#else
static zval *request_sapiupload_object_get_property_ptr_ptr(zval *object, zval *name, int type, void **cache_slot)
{
    return NULL;
}
#endif
/* }}} */

/* {{{ register_prop_handlers */
static inline void register_default_prop_handlers(const char *name, size_t name_length)
{
    register_prop_handlers(
        &SapiUpload_prop_handlers,
        name,
        name_length,
        request_readonly_has_property_handler,
        request_readonly_read_property_handler,
        request_sapiupload_object_default_write_property,
        request_sapiupload_object_default_unset_property
    );
}
/* }}} */

/* {{{ Argument Info */
ZEND_BEGIN_ARG_INFO_EX(SapiUpload_construct_args, 0, 0, 0)
    ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 1)
    ZEND_ARG_TYPE_INFO(0, type, IS_STRING, 1)
    ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 1)
    ZEND_ARG_TYPE_INFO(0, tmpName, IS_STRING, 1)
    ZEND_ARG_TYPE_INFO(0, error, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(SapiUpload_move_args, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, destination, IS_STRING, 0)
ZEND_END_ARG_INFO()
/* }}} Argument Info */

/* {{{ proto void SapiUpload::__construct([string $name, [string $type, [int $size, [string $tmpName, [int $error]]]]]) */
PHP_METHOD(SapiUpload, __construct)
{
    zval *_this_zval = getThis();
    zval *name = NULL;
    zval *type = NULL;
    zval *size = NULL;
    zval *tmpName = NULL;
    zval *error = NULL;
    zval *uninit = NULL;
    zval rv = {0};

    // needs to be zval to support null?
    ZEND_PARSE_PARAMETERS_START(0, 5)
        Z_PARAM_OPTIONAL
        Z_PARAM_ZVAL(name)
        Z_PARAM_ZVAL(type)
        Z_PARAM_ZVAL(size)
        Z_PARAM_ZVAL(tmpName)
        Z_PARAM_ZVAL(error)
    ZEND_PARSE_PARAMETERS_END();

    uninit = zend_read_property(SapiUpload_ce_ptr, _this_zval, ZEND_STRL("isUninitialized"), 0, &rv);
    if( !uninit || !zend_is_true(uninit) ) {
        zend_string *ce_name = Z_OBJCE_P(_this_zval)->name;
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "%.*s::__construct() called after construction.", (int)ZSTR_LEN(ce_name), ZSTR_VAL(ce_name));
        goto err;
    }
    zend_unset_property(SapiUpload_ce_ptr, _this_zval, ZEND_STRL("isUninitialized"));

    if (name) {
        zend_update_property(SapiUpload_ce_ptr, _this_zval, ZEND_STRL("name"), name);
    }
    if (type) {
        zend_update_property(SapiUpload_ce_ptr, _this_zval, ZEND_STRL("type"), type);
    }
    if (size) {
        zend_update_property(SapiUpload_ce_ptr, _this_zval, ZEND_STRL("size"), size);
    }
    if (tmpName) {
        zend_update_property(SapiUpload_ce_ptr, _this_zval, ZEND_STRL("tmpName"), tmpName);
    }
    if (error) {
        zend_update_property(SapiUpload_ce_ptr, _this_zval, ZEND_STRL("error"), error);
    }

err:
    zval_ptr_dtor(&rv);
}
/* }}} SapiUpload::__construct */

/* {{{ proto bool SapiUpload::move(string $destination) */
PHP_METHOD(SapiUpload, move)
{
    zval *_this_zval = getThis();
    zend_string *destination = NULL;
    zval *tmp_name = NULL;
    zval func_name = {0};
    zval params[2] = {0};
    zval rv = {0};
    zval *retval = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(destination)
    ZEND_PARSE_PARAMETERS_END();

    tmp_name = zend_read_property(SapiUpload_ce_ptr, _this_zval, ZEND_STRL("tmpName"), 0, &rv);
    if (!tmp_name) {
        RETURN_FALSE;
    }

    ZVAL_ZVAL(&params[0], tmp_name, 1, 0);
    ZVAL_STR(&params[1], destination);

    ZVAL_STRING(&func_name, "move_uploaded_file");

    retval = call_user_function(NULL, NULL, &func_name, return_value, 2, params);
    if (retval) {
        ZVAL_ZVAL(return_value, retval, 1, 0);
    }

    zval_ptr_dtor(&func_name);
    zval_ptr_dtor(&params[0]);
    zval_ptr_dtor(&params[1]);
    zval_ptr_dtor(&rv);
}
/* }}} SapiUpload::move */

/* {{{ SapiResponseSender methods */
static zend_function_entry SapiUpload_methods[] = {
    PHP_ME(SapiUpload, __construct, SapiUpload_construct_args, ZEND_ACC_PUBLIC)
    PHP_ME(SapiUpload, move, SapiUpload_move_args, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
/* }}} SapiResponseSender methods */

/* {{{ PHP_MINIT_FUNCTION */
PHP_REQUEST_API PHP_MINIT_FUNCTION(sapiupload)
{
    zend_class_entry SapiUpload_ce;

    zend_hash_init(&SapiUpload_prop_handlers, 0, NULL, NULL, 1);

    memcpy(&SapiUpload_obj_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    SapiUpload_obj_handlers.has_property = request_sapiupload_object_has_property;
    SapiUpload_obj_handlers.read_property = request_sapiupload_object_read_property;
    SapiUpload_obj_handlers.write_property = request_sapiupload_object_write_property;
    SapiUpload_obj_handlers.unset_property = request_sapiupload_object_unset_property;
    SapiUpload_obj_handlers.get_property_ptr_ptr = request_sapiupload_object_get_property_ptr_ptr;
    SapiUpload_obj_handlers.clone_obj = request_sapiupload_clone_obj;

    INIT_CLASS_ENTRY(SapiUpload_ce, "SapiUpload", SapiUpload_methods);
    SapiUpload_ce_ptr = zend_register_internal_class(&SapiUpload_ce);
    SapiUpload_ce_ptr->create_object = request_sapiupload_obj_create;

    zend_declare_property_bool(SapiUpload_ce_ptr, ZEND_STRL("isUninitialized"), 1, ZEND_ACC_PRIVATE);

    zend_declare_property_null(SapiUpload_ce_ptr, ZEND_STRL("name"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("name"));
    zend_declare_property_null(SapiUpload_ce_ptr, ZEND_STRL("type"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("type"));
    zend_declare_property_null(SapiUpload_ce_ptr, ZEND_STRL("size"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("size"));
    zend_declare_property_null(SapiUpload_ce_ptr, ZEND_STRL("tmpName"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("tmpName"));
    zend_declare_property_null(SapiUpload_ce_ptr, ZEND_STRL("error"), ZEND_ACC_PUBLIC);
    register_default_prop_handlers(ZEND_STRL("error"));

    return SUCCESS;
}
/* }}} PHP_MINIT_FUNCTION */

/* {{{ PHP_MSHUTDOWN_FUNCTION */
PHP_MSHUTDOWN_FUNCTION(sapiupload)
{
    zend_hash_destroy(&SapiUpload_prop_handlers);
    return SUCCESS;
}
/* }}} */
