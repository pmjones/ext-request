
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "main/php.h"
#include "main/php_streams.h"
#include "main/SAPI.h"
#include "ext/spl/spl_exceptions.h"
#include "ext/date/php_date.h"
#include "ext/standard/php_string.h"
#include "ext/standard/url.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_interfaces.h"
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

/* {{{ php_response_header */
static void php_response_header(zval *object, zend_string *label, zval *value, zend_bool replace)
{
    zval member;
    zval *prop_ptr;
    zval arr;
    zend_string *normal_label;
    zend_string *value_str;
    zend_string *tmp;
    zval *header_arr = NULL;

    // Read property pointer
    if( !Z_OBJ_HT_P(object)->get_property_ptr_ptr ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "PhpResponse::setHeader requires get_property_ptr_ptr");
        return;
    }

    ZVAL_STRING(&member, "headers");
    prop_ptr = Z_OBJ_HT_P(object)->get_property_ptr_ptr(object, &member, BP_VAR_RW, NULL);
    if( !prop_ptr || Z_TYPE_P(prop_ptr) != IS_ARRAY ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "PhpResponse::$headers must be an array");
        return;
    }

    // Normalize label
    normal_label = php_trim(label, ZEND_STRL(" \t\r\n\v"), 3);
    php_request_normalize_header_name(ZSTR_VAL(normal_label), ZSTR_LEN(normal_label));
    zend_string_forget_hash_val(normal_label);
    zend_string_hash_val(normal_label);

    if( !ZSTR_LEN(label) ) {
        zend_string_release(normal_label);
        return;
    }

    // Convert value to string
    if( Z_TYPE_P(value) == IS_ARRAY ) {
        tmp = array_to_csv(value);
    } else {
        tmp = zval_get_string(value);
    }
    value_str = php_trim(tmp, ZEND_STRL(" \t\r\n\v"), 3);
    zend_string_release(tmp);

    if( !ZSTR_LEN(value_str) ) {
        zend_string_release(normal_label);
        zend_string_release(value_str);
        return;
    }

    // Set value
    if( !replace ) {
        header_arr = zend_hash_find(Z_ARRVAL_P(prop_ptr), normal_label);
    }

    if( header_arr ) {
        add_next_index_str(header_arr, value_str);
    } else {
        array_init_size(&arr, 1);
        add_next_index_str(&arr, value_str);
        zend_hash_update(Z_ARRVAL_P(prop_ptr), normal_label, &arr);
    }

    zend_string_release(normal_label);
}
static inline void php_response_header_stringl(
    zval *object,
    const char *label, size_t label_len,
    const char *value, size_t value_len,
    zend_bool replace
) {
    zend_string *header_key = zend_string_init(label, label_len, 0);
    zval header_val = {0};

    ZVAL_STRINGL(&header_val, value, value_len);

    php_response_header(object, header_key, &header_val, 1);

    zval_ptr_dtor(&header_val);
    zend_string_release(header_key);
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
    if( retval ) {
        RETVAL_ZVAL(retval, 1, 0);
    }
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
}
/* }}} PhpResponse::getHeaders */

/* {{{ proto void PhpResponse::setHeader(string $label, mixed $value) */
PHP_METHOD(PhpResponse, setHeader)
{
    zend_string *label;
    zval *value;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(label)
        Z_PARAM_ZVAL(value)
    ZEND_PARSE_PARAMETERS_END();

    php_response_header(getThis(), label, value, 1);
}
/* }}} PhpResponse::setHeader */

/* {{{ proto void PhpResponse::addHeader(string $label, mixed $value) */
PHP_METHOD(PhpResponse, addHeader)
{
    zend_string *label;
    zval *value;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(label)
        Z_PARAM_ZVAL(value)
    ZEND_PARSE_PARAMETERS_END();

    php_response_header(getThis(), label, value, 0);
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
static inline void throw_json_exception()
{
    zval func_name;
    zval json_errmsg = {0};
    zval json_errno = {0};

    ZVAL_STRING(&func_name, "json_last_error_msg");
    call_user_function(EG(function_table), NULL, &func_name, &json_errmsg, 0, NULL);

    ZVAL_STRING(&func_name, "json_last_error");
    call_user_function(EG(function_table), NULL, &func_name, &json_errno, 0, NULL);

    convert_to_string(&json_errmsg);
    zend_throw_exception_ex(spl_ce_RuntimeException, zval_get_long(&json_errno),
                            "JSON encoding failed: %.*s", Z_STRLEN(json_errmsg), Z_STRVAL(json_errmsg));

    zval_ptr_dtor(&func_name);
    zval_ptr_dtor(&json_errmsg);
    zval_ptr_dtor(&json_errno);
}

PHP_METHOD(PhpResponse, setContentJson)
{
    zval *value;
    zend_long options = 0;
    zend_long depth = 512;

    zval *_this_zval = getThis();
    zval func_name = {0};
    zval json_value = {0};
    zval params[3];
    zend_bool failed = 0;

    ZEND_PARSE_PARAMETERS_START(1, 3)
        Z_PARAM_ZVAL(value)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(options)
        Z_PARAM_LONG(depth)
    ZEND_PARSE_PARAMETERS_END();

    // Check
    if( !zend_hash_str_exists(EG(function_table), "json_encode", sizeof("json_encode") - 1) ) {
        zend_throw_exception(spl_ce_RuntimeException, "json_encode() not available", 0);
        return;
    }

    // Call json_encode
    ZVAL_STRING(&func_name, "json_encode");
    ZVAL_ZVAL(&params[0], value, 1, 0);
    ZVAL_LONG(&params[1], options);
    ZVAL_LONG(&params[2], depth);

    call_user_function(EG(function_table), NULL, &func_name, &json_value, 3, params);

    if( Z_TYPE(json_value) != IS_STRING ) {
        failed = 1;
    } else {
        zend_update_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("content"), &json_value);
    }

    zval_ptr_dtor(&json_value);
    zval_ptr_dtor(&params[0]);
    zval_ptr_dtor(&params[1]);
    zval_ptr_dtor(&params[2]);
    zval_ptr_dtor(&func_name);

    if( failed ) {
        throw_json_exception();
        return;
    }

    // Set header
    php_response_header_stringl(_this_zval, ZEND_STRL("Content-Type"), ZEND_STRL("application/json"), 1);
}
/* }}} PhpResponse::setContentJson */

/* {{{ proto void PhpResponse::setContentResource(resource $fh [, string $disposition [, array $params = array()]]) */
PHP_METHOD(PhpResponse, setContentResource)
{
    zval *zstream;
    zend_string *disposition = NULL;
    zval *params = NULL;

    zval *_this_zval = getThis();

    ZEND_PARSE_PARAMETERS_START(1, 3)
        Z_PARAM_RESOURCE(zstream)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(disposition)
        Z_PARAM_ARRAY(params)
    ZEND_PARSE_PARAMETERS_END();

    // Set headers
    php_response_header_stringl(_this_zval, ZEND_STRL("Content-Type"), ZEND_STRL("application/octet-stream"), 1);
    php_response_header_stringl(_this_zval, ZEND_STRL("Content-Transfer-Encoding"), ZEND_STRL("binary"), 1);

    // Set disposition
    if( disposition ) {
        if( Z_TYPE_P(params) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(params)) ) {
            smart_str buf = {0};
            smart_str_append(&buf, disposition);
            smart_str_appendc(&buf, ';');
            _array_to_semicsv(&buf, params);
            smart_str_0(&buf);
            php_response_header_stringl(_this_zval, ZEND_STRL("Content-Disposition"), ZSTR_VAL(buf.s), ZSTR_LEN(buf.s), 1);
            smart_str_free(&buf);
        } else {
            php_response_header_stringl(_this_zval, ZEND_STRL("Content-Disposition"), ZSTR_VAL(disposition), ZSTR_LEN(disposition), 1);
        }
    }

    // Update content
    zend_update_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("content"), zstream);
}
/* }}} PhpResponse::setContentResource */

/* {{{ php_response_set_download */
static inline void php_response_set_download(INTERNAL_FUNCTION_PARAMETERS, zend_bool is_inline)
{
    zval *zstream;
    zend_string *name = NULL;
    zval *params = NULL;

    zval *_this_zval = getThis();
    zval func_name = {0};
    zval func_params[3] = {0};
    zval retval = {0};
    zend_string *tmp_filename;

    ZEND_PARSE_PARAMETERS_START(2, 3)
            Z_PARAM_RESOURCE(zstream)
            Z_PARAM_STR(name)
            Z_PARAM_OPTIONAL
            Z_PARAM_ARRAY_EX(params, 1, 0)
    ZEND_PARSE_PARAMETERS_END();

    // Make filename param
    tmp_filename = php_raw_url_encode(ZSTR_VAL(name), ZSTR_LEN(name));
    smart_str buf = {0};
    smart_str_appendc(&buf, '"');
    smart_str_append(&buf, tmp_filename);
    smart_str_appendc(&buf, '"');
    smart_str_0(&buf);

    // Make params
    ZVAL_ZVAL(&func_params[0], zstream, 1, 0);
    if( is_inline ) {
        ZVAL_STRING(&func_params[1], "inline");
    } else {
        ZVAL_STRING(&func_params[1], "attachment");
    }
    if( params ) {
        ZVAL_ZVAL(&func_params[2], params, 1, 0);
    } else {
        array_init(&func_params[2]);
    }
    add_assoc_str_ex(&func_params[2], ZEND_STRL("filename"), buf.s);

    // Call
    ZVAL_STRING(&func_name, "setContentResource");
    call_user_function(NULL, _this_zval, &func_name, &retval, 3, func_params);

    // Release
    zval_ptr_dtor(&func_name);
    zval_ptr_dtor(&func_params[2]);
    zval_ptr_dtor(&func_params[1]);
    zval_ptr_dtor(&func_params[0]);
    smart_str_free(&buf);
    zval_ptr_dtor(&retval);
    zend_string_release(tmp_filename);
}
/* }}} */

/* {{{ proto void PhpResponse::setDownload(resource $fh [, string $name [, array $params = array()]]) */
PHP_METHOD(PhpResponse, setDownload)
{
    php_response_set_download(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} PhpResponse::setDownload */

/* {{{ proto void PhpResponse::setDownloadInline(resource $fh [, string $name [, array $params = array()]]) */
PHP_METHOD(PhpResponse, setDownloadInline)
{
    php_response_set_download(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} PhpResponse::setDownloadInline */

/* {{{ proto string PhpResponse::date(mixed $date) */
PHP_METHOD(PhpResponse, date)
{
    zval *date_arg;
    zval *date;
    zval tmp = {0};
    zval ts = {0};
    zend_string *date_str;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(date_arg)
    ZEND_PARSE_PARAMETERS_END();

    // Create date object
    if( Z_TYPE_P(date_arg) == IS_OBJECT && instanceof_function(Z_OBJCE_P(date_arg), php_date_get_date_ce()) ) {
        date = date_arg;
    } else {
        object_init_ex(&tmp, php_date_get_date_ce());
        zend_call_method_with_1_params(&tmp, NULL, NULL, "__construct", NULL, date_arg);
        date = &tmp;
        if( EG(exception) ) {
            zend_object_store_ctor_failed(Z_OBJ(tmp));
            return;
        }
    }

    // Get timestamp
    zend_call_method_with_0_params(date, php_date_get_date_ce(), NULL, "gettimestamp", &ts);

    if( Z_TYPE(ts) == IS_LONG ) {
        // Format
        date_str = php_format_date(ZEND_STRL("D, d M Y H:i:s O"), Z_LVAL(ts), 0);
        RETVAL_STR(date_str);
    }

    zval_ptr_dtor(&tmp);
    zval_ptr_dtor(&ts);
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
