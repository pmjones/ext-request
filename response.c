
#ifdef HAVE_CONFIG_H

#include "config.h"
#endif

#include "main/php.h"
#include "main/php_streams.h"
#include "ext/spl/spl_exceptions.h"
#include "ext/standard/php_string.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_types.h"
#include "Zend/zend_smart_str.h"

#include "php_request.h"
#include "request_utils.h"

zend_class_entry * PhpResponse_ce_ptr;

/* {{{ Argument Info */
#define AI(method) PhpReponse_##method##_args

ZEND_BEGIN_ARG_INFO_EX(AI(__construct), 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(AI(getVersion), 0, 0, IS_STRING, NULL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(AI(setVersion), 0, 1, IS_NULL, NULL, 0)
    ZEND_ARG_TYPE_INFO(0, version, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(AI(getStatus), 0, 0, IS_LONG, NULL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(AI(setStatus), 0, 1, IS_NULL, NULL, 0)
    ZEND_ARG_TYPE_INFO(0, status, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(AI(getHeaders), 0, 0, IS_ARRAY, NULL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(AI(addSetHeader), 0, 2, IS_NULL, NULL, 0)
    ZEND_ARG_TYPE_INFO(0, label, IS_STRING, 0)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(AI(getCookies), 0, 0, IS_ARRAY, NULL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(AI(setCookie), 0, 1, IS_NULL, NULL, 0)
    ZEND_ARG_INFO(0, name)
    ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, expires, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, domain, IS_STRING, 0)
    ZEND_ARG_INFO(0, secure)
    ZEND_ARG_INFO(0, httponly)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(AI(getContent), 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(AI(setContent), 0, 1, IS_NULL, NULL, 0)
    ZEND_ARG_INFO(0, content)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(AI(setContentJson), 0, 1, IS_NULL, NULL, 0)
    ZEND_ARG_INFO(0, content)
    ZEND_ARG_TYPE_INFO(0, options, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, depth, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(AI(setContentResource), 0, 1, IS_NULL, NULL, 0)
    ZEND_ARG_TYPE_INFO(0, fh, IS_RESOURCE, 0)
    ZEND_ARG_TYPE_INFO(0, disposition, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, params, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(AI(setDownload), 0, 1, IS_NULL, NULL, 0)
    ZEND_ARG_TYPE_INFO(0, fh, IS_RESOURCE, 0)
    ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, params, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(AI(date), 0, 1, IS_STRING, NULL, 0)
    ZEND_ARG_INFO(0, date)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(AI(send), 0, 0, IS_NULL, NULL, 0)
ZEND_END_ARG_INFO()
/* }}} Argument Info */

/* {{{ Array to CSV */
static inline void _array_to_semicsv(smart_str *str, zval *arr)
{
    zend_string *key;
    zend_ulong index;
    zval *val;
    zend_bool first = 1;
    zend_string *tmp;

    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(arr), index, key, val) {
        if( first ) {
            first = 0;
        } else {
            smart_str_appendc(str, ';');
        }
        tmp = zval_get_string(val);
        if( key ) {
            smart_str_append(str, key);
            smart_str_appendc(str, '=');
        }
        smart_str_append(str, tmp);
        zend_string_release(tmp);
    } ZEND_HASH_FOREACH_END();
}

static inline void _array_to_csv(smart_str *str, zval *arr)
{
    zend_string *key;
    zend_ulong index;
    zval *val;
    zend_bool first = 1;
    zend_string *tmp;

    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(arr), index, key, val) {
        if( first ) {
            first = 0;
        } else {
            smart_str_appendl_ex(str, ZEND_STRL(", "), 0);
        }
        if( !key ) {
            tmp = zval_get_string(val);
            smart_str_append(str, tmp);
            zend_string_release(tmp);
        } else if( Z_TYPE_P(val) == IS_ARRAY ) {
            smart_str_append(str, key);
            smart_str_appendc(str, ';');
            _array_to_semicsv(str, val);
        } else {
            tmp = zval_get_string(val);
            smart_str_append(str, key);
            smart_str_appendc(str, '=');
            smart_str_append(str, tmp);
            zend_string_release(tmp);
        }
    } ZEND_HASH_FOREACH_END();
}

static inline zend_string *array_to_semicsv(zval *arr)
{
    smart_str str = {0};
    _array_to_semicsv(&str, arr);
    smart_str_0(&str);
    return str.s;
}

static inline zend_string *array_to_csv(zval *arr)
{
    smart_str str = {0};
    _array_to_csv(&str, arr);
    smart_str_0(&str);
    return str.s;
}
/* }}} */

/* {{{ proto PhpResponse::__construct() */
PHP_METHOD(PhpResponse, __construct)
{
    zval *_this_zval = getThis();
    zval arr;

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    array_init(&arr);
    zend_update_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("headers"), &arr);
    array_init(&arr);
    zend_update_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("cookies"), &arr);
}
/* }}} PhpResponse::getVersion */

/* {{{ proto string PhpResponse::getVersion() */
PHP_METHOD(PhpResponse, getVersion)
{
    zval *_this_zval = getThis();
    zval *retval;

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    retval = zend_read_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("version"), 0, NULL);
    RETVAL_ZVAL(retval, 1, 0);
}
/* }}} PhpResponse::getVersion */

/* {{{ proto void PhpResponse::setVersion(string $version) */
PHP_METHOD(PhpResponse, setVersion)
{
    zval *_this_zval = getThis();
    zend_string *version;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(version)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property_str(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("version"), version);
}
/* }}} PhpResponse::setVersion */

/* {{{ proto int PhpResponse::getStatus() */
PHP_METHOD(PhpResponse, getStatus)
{
    zval *_this_zval = getThis();
    zval *retval;

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    retval = zend_read_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("status"), 0, NULL);
    if( retval ) {
        RETVAL_ZVAL(retval, 1, 0);
    }
    convert_to_long(return_value);
}
/* }}} PhpResponse::getStatus */

/* {{{ proto void PhpResponse::setStatus(int $version) */
PHP_METHOD(PhpResponse, setStatus)
{
    zval *_this_zval = getThis();
    zend_long status;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(status)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property_long(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("status"), status);
}
/* }}} PhpResponse::setStatus */

/* {{{ proto array PhpResponse::getHeaders() */
PHP_METHOD(PhpResponse, getHeaders)
{
    zval *_this_zval = getThis();
    zval *retval;

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    retval = zend_read_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("headers"), 0, NULL);
    if( retval ) {
        RETVAL_ZVAL(retval, 1, 0);
    }
    convert_to_array(return_value);
}
/* }}} PhpResponse::getHeaders */

/* {{{ proto void PhpResponse::setHeader(string $label, mixed $value) */
PHP_METHOD(PhpResponse, setHeader)
{
    zend_string *label;
    zval *value;

    zval *_this_zval = getThis();
    zval member;
    zval *prop_ptr;
    zend_string *tmp;
    zend_string *value_str;
    zval arr;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(label)
        Z_PARAM_ZVAL(value)
    ZEND_PARSE_PARAMETERS_END();

    // Read property pointer
    if( !Z_OBJ_HT_P(_this_zval)->get_property_ptr_ptr ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "PhpResponse::setHeader requires get_property_ptr_ptr");
        return;
    }
    ZVAL_STRING(&member, "headers");
    prop_ptr = Z_OBJ_HT_P(_this_zval)->get_property_ptr_ptr(_this_zval, &member, BP_VAR_RW, NULL);
    if( !prop_ptr || Z_TYPE_P(prop_ptr) != IS_ARRAY ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "PhpResponse::$headers must be an array");
        return;
    }

    // Dup and mutate string
    tmp = zend_string_dup(label, 0);
    php_request_normalize_header_name(ZSTR_VAL(tmp), ZSTR_LEN(tmp));
    zend_string_forget_hash_val(tmp);
    label = tmp;

    tmp = php_trim(label, ZEND_STRL(" \t\r\n\v"), 3);
    zend_string_release(label);
    label = tmp;

    if( ZSTR_LEN(label) ) {
        // Convert to string and trim
        if( Z_TYPE_P(value) == IS_ARRAY ) {
            tmp = array_to_csv(value);
        } else {
            tmp = zval_get_string(value);
        }
        value_str = php_trim(tmp, ZEND_STRL(" \t\r\n\v"), 3);
        zend_string_release(tmp);

        // Add to headers array
        if( ZSTR_LEN(value_str) ) {
            array_init_size(&arr, 1);
            add_next_index_str(&arr, value_str);
            add_assoc_zval_ex(prop_ptr, ZSTR_VAL(label), ZSTR_LEN(label), &arr);
        } else {
            // @todo unset
            zend_string_release(value_str);
        }
    }

    zend_string_release(label);
}
/* }}} PhpResponse::setHeader */

/* {{{ proto void PhpResponse::addHeader(string $label, mixed $value) */
PHP_METHOD(PhpResponse, addHeader)
{
    zend_string *label;
    zval *value;

    zval *_this_zval = getThis();
    zval member;
    zval *prop_ptr;
    zend_string *tmp;
    zend_string *value_str;
    zval arr;
    zval *header_arr;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(label)
        Z_PARAM_ZVAL(value)
    ZEND_PARSE_PARAMETERS_END();

    // Read property pointer
    if( !Z_OBJ_HT_P(_this_zval)->get_property_ptr_ptr ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "PhpResponse::setHeader requires get_property_ptr_ptr");
        return;
    }
    ZVAL_STRING(&member, "headers");
    prop_ptr = Z_OBJ_HT_P(_this_zval)->get_property_ptr_ptr(_this_zval, &member, BP_VAR_RW, NULL);
    if( !prop_ptr || Z_TYPE_P(prop_ptr) != IS_ARRAY ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "PhpResponse::$headers must be an array");
        return;
    }

    // Dup and mutate string
    tmp = zend_string_dup(label, 0);
    php_request_normalize_header_name(ZSTR_VAL(tmp), ZSTR_LEN(tmp));
    zend_string_forget_hash_val(tmp);
    label = tmp;

    tmp = php_trim(label, ZEND_STRL(" \t\r\n\v"), 3);
    zend_string_release(label);
    label = tmp;

    if( ZSTR_LEN(label) ) {
        // Convert to string and trim
        if( Z_TYPE_P(value) == IS_ARRAY ) {
            tmp = array_to_csv(value);
        } else {
            tmp = zval_get_string(value);
        }
        value_str = php_trim(tmp, ZEND_STRL(" \t\r\n\v"), 3);
        zend_string_release(tmp);

        // Add to headers array
        if( ZSTR_LEN(value_str) ) {
            header_arr = zend_hash_find(Z_ARRVAL_P(prop_ptr), label);
            if( header_arr ) {
                add_next_index_str(header_arr, value_str);
            } else {
                array_init_size(&arr, 1);
                add_next_index_str(&arr, value_str);
                zend_hash_update(Z_ARRVAL_P(prop_ptr), label, &arr);
            }
        } else {
            zend_string_release(value_str);
        }
    }

    zend_string_release(label);
}
/* }}} PhpResponse::addHeader */

/* {{{ proto array PhpResponse::getCookies() */
PHP_METHOD(PhpResponse, getCookies)
{
    zval *_this_zval = getThis();
    zval *retval;

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    retval = zend_read_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("cookies"), 0, NULL);
    if( retval ) {
        RETVAL_ZVAL(retval, 1, 0);
    }
    convert_to_array(return_value);
}
/* }}} PhpResponse::getCookies */

/* {{{ proto void PhpResponse::setCookie(string name [, string value [, int expires [, string path [, string domain [, bool secure[, bool httponly]]]]]]) */
static void php_response_setcookie(INTERNAL_FUNCTION_PARAMETERS, zend_bool raw)
{
    zend_string *name;
    zend_string *value = NULL;
    zend_long expires = 0;
    zend_string *path = NULL;
    zend_string *domain = NULL;
    zend_bool secure = 0;
    zend_bool httponly = 0;

    zval *_this_zval = getThis();
    zval *ptr;
    zval member;
    zval cookie;

    ZEND_PARSE_PARAMETERS_START(1, 7)
        Z_PARAM_STR(name)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(value)
        Z_PARAM_LONG(expires)
        Z_PARAM_STR(path)
        Z_PARAM_STR(domain)
        Z_PARAM_BOOL(secure)
        Z_PARAM_BOOL(httponly)
    ZEND_PARSE_PARAMETERS_END();

    if( !Z_OBJ_HT_P(_this_zval)->get_property_ptr_ptr ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "PhpResponse::setCookie requires get_property_ptr_ptr");
        return;
    }

    // Read property pointer
    ZVAL_STRING(&member, "cookies");
    ptr = Z_OBJ_HT_P(_this_zval)->get_property_ptr_ptr(_this_zval, &member, BP_VAR_RW, NULL);

    if( !ptr ) {
        // fall-through
    } else if( Z_TYPE_P(ptr) != IS_ARRAY ) {
        convert_to_array(ptr);
    }

    // Make cookies array
    array_init_size(&cookie, 7);
    add_assoc_bool_ex(&cookie, ZEND_STRL("raw"), raw);
    if( value ) {
        add_assoc_str_ex(&cookie, ZEND_STRL("value"), value);
    } else {
        add_assoc_stringl_ex(&cookie, ZEND_STRL("value"), ZEND_STRL(""));
    }
    add_assoc_long_ex(&cookie, ZEND_STRL("expire"), expires);
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

    // Update property
    if( ptr ) {
        add_assoc_zval_ex(ptr, ZSTR_VAL(name), ZSTR_LEN(name), &cookie);
    } else {
        zend_update_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("cookies"), &cookie);
    }

    // Cleanup
    zval_dtor(&member);
}

PHP_METHOD(PhpResponse, setCookie)
{
    php_response_setcookie(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} PhpResponse::setCookie */

/* {{{ proto void PhpResponse::setRawCookie(string name [, string value [, int expires [, string path [, string domain [, bool secure[, bool httponly]]]]]]) */
PHP_METHOD(PhpResponse, setRawCookie)
{
    php_response_setcookie(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} PhpResponse::setRawCookie */

/* {{{ proto mixed PhpResponse::getContent() */
PHP_METHOD(PhpResponse, getContent)
{
    zval *_this_zval = getThis();
    zval *retval;

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    retval = zend_read_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("content"), 0, NULL);
    if( retval ) {
        RETVAL_ZVAL(retval, 1, 0);
    }
}
/* }}} PhpResponse::getContent */

/* {{{ proto void PhpResponse::setContent(mixed $content) */
PHP_METHOD(PhpResponse, setContent)
{
    zval *_this_zval = getThis();
    zval *content;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(content)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("content"), content);
}
/* }}} PhpResponse::setContent */

/* {{{ proto void PhpResponse::setContentJson(mixed $value [, int $options [, int $depth = 512]]) */
PHP_METHOD(PhpResponse, setContentJson)
{
    zval *value;
    zend_long options = 0;
    zend_long depth = 512;

    ZEND_PARSE_PARAMETERS_START(1, 3)
        Z_PARAM_ZVAL(value)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(options)
        Z_PARAM_LONG(depth)
    ZEND_PARSE_PARAMETERS_END();
}
/* }}} PhpResponse::setContentJson */

/* {{{ proto void PhpResponse::setContentResource(resource $fh [, string $disposition [, array $params = array()]]) */
PHP_METHOD(PhpResponse, setContentResource)
{
    zval *zstream;
    zend_string *disposition = NULL;
    zval *params = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 3)
        Z_PARAM_RESOURCE(zstream)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(disposition)
        Z_PARAM_ARRAY(params)
    ZEND_PARSE_PARAMETERS_END();
}
/* }}} PhpResponse::setContentResource */

/* {{{ proto void PhpResponse::setDownload(resource $fh [, string $name [, array $params = array()]]) */
PHP_METHOD(PhpResponse, setDownload)
{
    zval *zstream;
    zend_string *name = NULL;
    zval *params = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 3)
        Z_PARAM_RESOURCE(zstream)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(name)
        Z_PARAM_ARRAY(params)
    ZEND_PARSE_PARAMETERS_END();
}
/* }}} PhpResponse::setDownload */

/* {{{ proto void PhpResponse::setDownloadInline(resource $fh [, string $name [, array $params = array()]]) */
PHP_METHOD(PhpResponse, setDownloadInline)
{
    zval *zstream;
    zend_string *name = NULL;
    zval *params = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 3)
        Z_PARAM_RESOURCE(zstream)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(name)
        Z_PARAM_ARRAY(params)
    ZEND_PARSE_PARAMETERS_END();
}
/* }}} PhpResponse::setDownloadInline */

/* {{{ proto string PhpResponse::date(mixed $date) */
PHP_METHOD(PhpResponse, date)
{
    zval *date;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(date)
    ZEND_PARSE_PARAMETERS_END();
}
/* }}} PhpResponse::date */

/* {{{ proto void PhpResponse::send() */
PHP_METHOD(PhpResponse, send)
{
    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();
}
/* }}} PhpResponse::send */

/* {{{ PhpResponse methods */
static zend_function_entry PhpResponse_methods[] = {
    PHP_ME(PhpResponse, __construct, AI(__construct), ZEND_ACC_PUBLIC)
    PHP_ME(PhpResponse, getVersion, AI(getVersion), ZEND_ACC_PUBLIC)
    PHP_ME(PhpResponse, setVersion, AI(setVersion), ZEND_ACC_PUBLIC)
    PHP_ME(PhpResponse, getStatus, AI(getStatus), ZEND_ACC_PUBLIC)
    PHP_ME(PhpResponse, setStatus, AI(setStatus), ZEND_ACC_PUBLIC)
    PHP_ME(PhpResponse, getHeaders, AI(getHeaders), ZEND_ACC_PUBLIC)
    PHP_ME(PhpResponse, setHeader, AI(addSetHeader), ZEND_ACC_PUBLIC)
    PHP_ME(PhpResponse, addHeader, AI(addSetHeader), ZEND_ACC_PUBLIC)
    PHP_ME(PhpResponse, getCookies, AI(getCookies), ZEND_ACC_PUBLIC)
    PHP_ME(PhpResponse, setCookie, AI(setCookie), ZEND_ACC_PUBLIC)
    PHP_ME(PhpResponse, setRawCookie, AI(setCookie), ZEND_ACC_PUBLIC)
    PHP_ME(PhpResponse, getContent, AI(getContent), ZEND_ACC_PUBLIC)
    PHP_ME(PhpResponse, setContent, AI(setContent), ZEND_ACC_PUBLIC)
    PHP_ME(PhpResponse, setContentJson, AI(setContentJson), ZEND_ACC_PUBLIC)
    PHP_ME(PhpResponse, setContentResource, AI(setContentResource), ZEND_ACC_PUBLIC)
    PHP_ME(PhpResponse, setDownload, AI(setDownload), ZEND_ACC_PUBLIC)
    PHP_ME(PhpResponse, setDownloadInline, AI(setDownload), ZEND_ACC_PUBLIC)
    PHP_ME(PhpResponse, date, AI(date), ZEND_ACC_PUBLIC)
    PHP_ME(PhpResponse, send, AI(send), ZEND_ACC_PUBLIC)
    PHP_FE_END
};
/* }}} PhpResponse methods */

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(response)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "PhpResponse", PhpResponse_methods);
    PhpResponse_ce_ptr = zend_register_internal_class(&ce);

    zend_declare_property_stringl(PhpResponse_ce_ptr, ZEND_STRL("version"), ZEND_STRL("1.1"), ZEND_ACC_PROTECTED);
    zend_declare_property_long(PhpResponse_ce_ptr, ZEND_STRL("status"), 200, ZEND_ACC_PROTECTED);
    zend_declare_property_null(PhpResponse_ce_ptr, ZEND_STRL("headers"), ZEND_ACC_PROTECTED);
    zend_declare_property_null(PhpResponse_ce_ptr, ZEND_STRL("cookies"), ZEND_ACC_PROTECTED);
    zend_declare_property_null(PhpResponse_ce_ptr, ZEND_STRL("content"), ZEND_ACC_PROTECTED);

    return SUCCESS;
}
/* }}} PHP_MINIT_FUNCTION */
