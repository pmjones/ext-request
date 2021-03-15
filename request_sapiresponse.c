
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Zend/zend_API.h"

#include "main/php.h"

#include "php_request.h"
#include "request_utils.h"
#include "request_sapiresponseinterface.h"

PHP_REQUEST_API zend_class_entry *SapiResponse_ce_ptr;

/* {{{ proto string SapiResponse::getVersion() */
static zval *sapi_response_get_version(zval *response)
{
    return php7to8_zend_read_property(SapiResponse_ce_ptr, response, ZEND_STRL("version"), 0, NULL);
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

#if PHP_VERSION_ID < 80000
    zend_update_property_str(SapiResponse_ce_ptr, _this_zval, ZEND_STRL("version"), version);
#else
    zend_update_property_str(SapiResponse_ce_ptr, Z_OBJ_P(_this_zval), ZEND_STRL("version"), version);
#endif

    RETURN_ZVAL(_this_zval, 1, 0);
}
/* }}} SapiResponse::setVersion */

/* {{{ proto int SapiResponse::getCode() */
static zval *sapi_response_get_code(zval *response)
{
    return php7to8_zend_read_property(SapiResponse_ce_ptr, response, ZEND_STRL("code"), 0, NULL);
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

#if PHP_VERSION_ID < 80000
    zend_update_property_long(SapiResponse_ce_ptr, _this_zval, ZEND_STRL("code"), code);
#else
    zend_update_property_long(SapiResponse_ce_ptr, Z_OBJ_P(_this_zval), ZEND_STRL("code"), code);
#endif

    RETURN_ZVAL(_this_zval, 1, 0);
}
/* }}} SapiResponse::setCode */

/* {{{ proto array SapiResponse::getHeaders() */
static zval *sapi_response_get_headers(zval *response)
{
    return php7to8_zend_read_property(SapiResponse_ce_ptr, response, ZEND_STRL("headers"), 0, NULL);
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

    headers = php7to8_zend_read_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("headers"), 0, NULL);
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

    headers = php7to8_zend_read_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("headers"), 0, NULL);
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

    php7to8_zend_update_property_null(SapiResponse_ce_ptr, _this_zval, ZEND_STRL("headers"));

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

    cookies = php7to8_zend_read_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("cookies"), 0, NULL);
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
    return php7to8_zend_read_property(SapiResponse_ce_ptr, response, ZEND_STRL("cookies"), 0, NULL);
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

    cookies = php7to8_zend_read_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("cookies"), 0, NULL);
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
        php7to8_zend_update_property(SapiResponse_ce_ptr, response, ZEND_STRL("cookies"), &cookie);
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

    php7to8_zend_update_property_null(SapiResponse_ce_ptr, _this_zval, ZEND_STRL("cookies"));

    RETURN_ZVAL(_this_zval, 1, 0);
}
/* }}} SapiResponse::unsetHeaders */

/* {{{ proto mixed SapiResponse::getContent() */
static zval *sapi_response_get_content(zval *response)
{
    return php7to8_zend_read_property(SapiResponse_ce_ptr, response, ZEND_STRL("content"), 0, NULL);
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

    php7to8_zend_update_property(SapiResponse_ce_ptr, _this_zval, ZEND_STRL("content"), content);

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
    php7to8_zend_update_property(SapiResponse_ce_ptr, _this_zval, ZEND_STRL("callbacks"), &arr);

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
    return php7to8_zend_read_property(SapiResponse_ce_ptr, response, ZEND_STRL("callbacks"), 0, NULL);
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
