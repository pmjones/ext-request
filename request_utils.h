
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_interfaces.h"
#include "Zend/zend_portability.h"
#include "Zend/zend_smart_str.h"
#include "Zend/zend_types.h"

#include "ext/standard/php_string.h"

#include "ext/spl/spl_exceptions.h"



#if PHP_VERSION_ID < 80000
#define php7to8_zend_update_property(ce, o, ...) zend_update_property(ce, o, __VA_ARGS__)
#define php7to8_zend_update_property_null(ce, o, ...) zend_update_property_null(ce, o, __VA_ARGS__)
#define php7to8_zend_read_property(ce, o, ...) zend_read_property(ce, o, __VA_ARGS__)
#else
#define php7to8_zend_update_property(ce, o, ...) zend_update_property(ce, Z_OBJ_P(o), __VA_ARGS__)
#define php7to8_zend_update_property_null(ce, o, ...) zend_update_property_null(ce, Z_OBJ_P(o), __VA_ARGS__)
#define php7to8_zend_read_property(ce, o, ...) zend_read_property(ce, Z_OBJ_P(o), __VA_ARGS__)
#endif



/* {{{ smart_str_appendz */
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
/* }}} smart_str_appendz */



/* {{{ sapi_request_normalize_header_name */
static inline void sapi_request_normalize_header_name(char *key, size_t key_length)
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

static inline zend_string *sapi_request_normalize_header_name_ex(zend_string *in)
{
    zend_string *out = php_trim(in, ZEND_STRL(" \t\r\n\v"), 3);
    sapi_request_normalize_header_name(ZSTR_VAL(out), ZSTR_LEN(out));
    zend_string_forget_hash_val(out);
    zend_string_hash_val(out);
    return out;
}
/* }}} sapi_request_normalize_header_name */



/* {{{ prop_handlers */
struct prop_handlers {
    zend_object_has_property_t has_property;
    zend_object_read_property_t read_property;
    zend_object_write_property_t write_property;
    zend_object_unset_property_t unset_property;
};

static inline void register_prop_handlers(
    HashTable *prop_handlers,
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
    zend_hash_str_update_mem(prop_handlers, name, name_length, &hnd, sizeof(hnd));
}
/* }}} prop_handlers */



/* {{{ sapi_request_throw_readonly_exception */
static inline void sapi_request_throw_readonly_exception(zend_object *object, zend_string *member)
{
    zend_string *ce_name = object->ce->name;
    zend_throw_exception_ex(spl_ce_RuntimeException, 0, "%.*s::$%.*s is read-only.", (int)ZSTR_LEN(ce_name), ZSTR_VAL(ce_name), (int)ZSTR_LEN(member), ZSTR_VAL(member));
}
/* }}} */



/* {{{ request_has_property_dispatcher */
#if PHP_MAJOR_VERSION >= 8
static inline int request_has_property_dispatcher(HashTable *handlers, zend_object *object, zend_string *member, int has_set_exists, void **cache_slot)
{
    struct prop_handlers *hnd = zend_hash_str_find_ptr(handlers, ZSTR_VAL(member), ZSTR_LEN(member));
    return (hnd ? hnd->has_property : std_object_handlers.has_property)(object, member, has_set_exists, cache_slot);
}
#else
static inline int request_has_property_dispatcher(HashTable *handlers, zval *object, zval *member, int has_set_exists, void **cache_slot)
{
    struct prop_handlers *hnd = zend_hash_str_find_ptr(handlers, Z_STRVAL_P(member), Z_STRLEN_P(member));
    return (hnd ? hnd->has_property : std_object_handlers.has_property)(object, member, has_set_exists, cache_slot);
}
#endif
/* }}} */



/* {{{ request_read_property_dispatcher */
#if PHP_MAJOR_VERSION >= 8
static inline zval *request_read_property_dispatcher(HashTable *handlers, zend_object *object, zend_string *member, int type, void **cache_slot, zval *rv)
{
    if( !object->ce->__get && !std_object_handlers.has_property(object, member, 2, cache_slot) ) {
        zend_string *ce_name = object->ce->name;
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "%.*s::$%.*s does not exist.", (int)ZSTR_LEN(ce_name), ZSTR_VAL(ce_name), (int)ZSTR_LEN(member), ZSTR_VAL(member));
        ZVAL_NULL(rv);
        return rv;
    }
    struct prop_handlers *hnd = zend_hash_str_find_ptr(handlers, ZSTR_VAL(member), ZSTR_LEN(member));
    return (hnd ? hnd->read_property : std_object_handlers.read_property)(object, member, type, cache_slot, rv);
}
#else
static inline zval *request_read_property_dispatcher(HashTable *handlers, zval *object, zval *member, int type, void **cache_slot, zval *rv)
{
    if( !Z_OBJCE_P(object)->__get && !std_object_handlers.has_property(object, member, 2, cache_slot) ) {
        zend_string *ce_name = Z_OBJCE_P(object)->name;
        zend_string *member_str = zval_get_string(member);
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "%.*s::$%.*s does not exist.", (int)ZSTR_LEN(ce_name), ZSTR_VAL(ce_name), (int)ZSTR_LEN(member_str), ZSTR_VAL(member_str));
        zend_string_release(member_str);
        ZVAL_NULL(rv);
        return rv;
    }
    struct prop_handlers *hnd = zend_hash_str_find_ptr(handlers, Z_STRVAL_P(member), Z_STRLEN_P(member));
    return (hnd ? hnd->read_property : std_object_handlers.read_property)(object, member, type, cache_slot, rv);
}
#endif
/* }}} */



/* {{{ request_write_property_dispatcher */
#if PHP_MAJOR_VERSION >= 8
static inline zval *request_write_property_dispatcher(HashTable *handlers, zend_object *object, zend_string *member, zval *value, void **cache_slot)
{
    if( !object->ce->__get && !std_object_handlers.has_property(object, member, 2, cache_slot) ) {
        zend_string *ce_name = object->ce->name;
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "%.*s::$%.*s does not exist.", (int)ZSTR_LEN(ce_name), ZSTR_VAL(ce_name), (int)ZSTR_LEN(member), ZSTR_VAL(member));
        return NULL;
    }

    if( !object->ce->__set && !std_object_handlers.has_property(object, member, 2, cache_slot) ) {
        sapi_request_throw_readonly_exception(object, member);
        return NULL;
    }

    struct prop_handlers *hnd = zend_hash_str_find_ptr(handlers, ZSTR_VAL(member), ZSTR_LEN(member));
    return (hnd ? hnd->write_property : std_object_handlers.write_property)(object, member, value, cache_slot);
}
#elif PHP_MAJOR_VERSION >= 7 && PHP_MINOR_VERSION >= 4
static inline zval *request_write_property_dispatcher(HashTable *handlers, zval *object, zval *member, zval *value, void **cache_slot)
{
    if( !Z_OBJCE_P(object)->__get && !std_object_handlers.has_property(object, member, 2, cache_slot) ) {
        zend_string *ce_name = Z_OBJCE_P(object)->name;
        zend_string *member_str = zval_get_string(member);
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "%.*s::$%.*s does not exist.", (int)ZSTR_LEN(ce_name), ZSTR_VAL(ce_name), (int)ZSTR_LEN(member_str), ZSTR_VAL(member_str));
        zend_string_release(member_str);
        return NULL;
    }

    if( !Z_OBJCE_P(object)->__set && !std_object_handlers.has_property(object, member, 2, cache_slot) ) {
        sapi_request_throw_readonly_exception(Z_OBJ_P(object), Z_STR_P(member));
        return NULL;
    }

    struct prop_handlers *hnd = zend_hash_str_find_ptr(handlers, Z_STRVAL_P(member), Z_STRLEN_P(member));
    return (hnd ? hnd->write_property : std_object_handlers.write_property)(object, member, value, cache_slot);
}
#else
static inline void request_write_property_dispatcher(HashTable *handlers, zval *object, zval *member, zval *value, void **cache_slot)
{
    if( !Z_OBJCE_P(object)->__get && !std_object_handlers.has_property(object, member, 2, cache_slot) ) {
        zend_string *ce_name = Z_OBJCE_P(object)->name;
        zend_string *member_str = zval_get_string(member);
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "%.*s::$%.*s does not exist.", (int)ZSTR_LEN(ce_name), ZSTR_VAL(ce_name), (int)ZSTR_LEN(member_str), ZSTR_VAL(member_str));
        zend_string_release(member_str);
        return;
    }

    if( !Z_OBJCE_P(object)->__set && !std_object_handlers.has_property(object, member, 2, cache_slot) ) {
        sapi_request_throw_readonly_exception(Z_OBJ_P(object), Z_STR_P(member));
        return;
    }

    struct prop_handlers *hnd = zend_hash_str_find_ptr(handlers, Z_STRVAL_P(member), Z_STRLEN_P(member));
    (hnd ? hnd->write_property : std_object_handlers.write_property)(object, member, value, cache_slot);
}
#endif
/* }}} */



/* {{{ request_unset_property_dispatcher */
#if PHP_MAJOR_VERSION >= 8
static void request_unset_property_dispatcher(HashTable *handlers, zend_object *object, zend_string *member, void **cache_slot)
{
    struct prop_handlers *hnd = zend_hash_str_find_ptr(handlers, ZSTR_VAL(member), ZSTR_LEN(member));
    return (hnd ? hnd->unset_property : std_object_handlers.unset_property)(object, member, cache_slot);
}
#else
static void request_unset_property_dispatcher(HashTable *handlers, zval *object, zval *member, void **cache_slot)
{
    struct prop_handlers *hnd = zend_hash_str_find_ptr(handlers, Z_STRVAL_P(member), Z_STRLEN_P(member));
    return (hnd ? hnd->unset_property : std_object_handlers.unset_property)(object, member, cache_slot);
}
#endif
/* }}} */




/* {{{ request_readonly_has_property_handler */
#if PHP_MAJOR_VERSION >= 8
static int request_readonly_has_property_handler(zend_object *object, zend_string *member, int check_empty, void **cache_slot)
{
    return 1;
}
#else
static int request_readonly_has_property_handler(zval *object, zval *member, int check_empty, void **cache_slot)
{
    return 1;
}
#endif
/* }}} */



/* {{{ request_readonly_read_property_handler */
#if PHP_MAJOR_VERSION >= 8
static zval *request_readonly_read_property_handler(zend_object *object, zend_string *member, int type, void **cache_slot, zval *rv)
{
    zval *retval;

    ZVAL_UNDEF(rv);
    retval = std_object_handlers.read_property(object, member, type, cache_slot, rv);

    // Make sure the property can't be modified
    if( !Z_ISREF_P(rv) && (type == BP_VAR_W || type == BP_VAR_RW  || type == BP_VAR_UNSET) ) {
        SEPARATE_ZVAL(rv);
        sapi_request_throw_readonly_exception(object, member);
    }

    return retval;
}
#else
static zval *request_readonly_read_property_handler(zval *object, zval *member, int type, void **cache_slot, zval *rv)
{
    zval *retval;

    ZVAL_UNDEF(rv);
    retval = std_object_handlers.read_property(object, member, type, cache_slot, rv);

    // Make sure the property can't be modified
    if( !Z_ISREF_P(rv) && (type == BP_VAR_W || type == BP_VAR_RW  || type == BP_VAR_UNSET) ) {
        SEPARATE_ZVAL(rv);
        sapi_request_throw_readonly_exception(Z_OBJ_P(object), Z_STR_P(member));
    }

    return retval;
}
#endif
/* }}} */



/* {{{ request_readonly_write_property_handler */
#if PHP_MAJOR_VERSION >= 8
static inline zval *request_readonly_write_property_handler(zend_class_entry *scope, zend_object *object, zend_string *member, zval *value, void **cache_slot)
{
    if( zend_get_executed_scope() != scope ) {
        sapi_request_throw_readonly_exception(object, member);
        return NULL;
    } else {
        return std_object_handlers.write_property(object, member, value, cache_slot);
    }
}
#elif PHP_MAJOR_VERSION >= 7 && PHP_MINOR_VERSION >= 4
static inline zval *request_readonly_write_property_handler(zend_class_entry *scope, zval *object, zval *member, zval *value, void **cache_slot)
{
    if( zend_get_executed_scope() != scope ) {
        sapi_request_throw_readonly_exception(Z_OBJ_P(object), Z_STR_P(member));
        return NULL;
    } else {
        return std_object_handlers.write_property(object, member, value, cache_slot);
    }
}
#else
static inline void request_readonly_write_property_handler(zend_class_entry *scope, zval *object, zval *member, zval *value, void **cache_slot)
{
    if( zend_get_executed_scope() != scope ) {
        sapi_request_throw_readonly_exception(Z_OBJ_P(object), Z_STR_P(member));
    } else {
        std_object_handlers.write_property(object, member, value, cache_slot);
    }
}
#endif
/* }}} */



/* {{{ request_readonly_unset_property_handler */
#if PHP_MAJOR_VERSION >= 8
static inline void request_readonly_unset_property_handler(zend_class_entry *scope, zend_object *object, zend_string *member, void **cache_slot)
{
    if( zend_get_executed_scope() != scope ) {
        sapi_request_throw_readonly_exception(object, member);
    } else {
        std_object_handlers.unset_property(object, member, cache_slot);
    }
}
#else
static inline void request_readonly_unset_property_handler(zend_class_entry *scope, zval *object, zval *member, void **cache_slot)
{
    if( zend_get_executed_scope() != scope ) {
        sapi_request_throw_readonly_exception(Z_OBJ_P(object), Z_STR_P(member));
    } else {
        std_object_handlers.unset_property(object, member, cache_slot);
    }
}
#endif
/* }}} */
