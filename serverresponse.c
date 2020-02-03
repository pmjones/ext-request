
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "main/php.h"
#include "main/php_output.h"
#include "main/php_streams.h"
#include "main/SAPI.h"
#include "main/snprintf.h"
#include "ext/spl/spl_exceptions.h"
#include "ext/standard/head.h"
#include "ext/standard/php_string.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_interfaces.h"
#include "Zend/zend_types.h"
#include "Zend/zend_smart_str.h"

#include "php_request.h"

extern zend_string *server_request_normalize_header_name_ex(zend_string *in);

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

zend_class_entry *ServerResponse_ce_ptr;

/* {{{ Argument Info */
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

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponse_addSetHeader_args, 0, 2, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, label, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
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
    PHP_ME(ServerResponse, getVersion, ServerResponse_getVersion_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, setVersion, ServerResponse_setVersion_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, getCode, ServerResponse_getCode_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, setCode, ServerResponse_setCode_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, getHeaders, ServerResponse_getHeaders_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, setHeader, ServerResponse_addSetHeader_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, addHeader, ServerResponse_addSetHeader_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, getCookies, ServerResponse_getCookies_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, setCookie, ServerResponse_setCookie_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, setRawCookie, ServerResponse_setCookie_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, getContent, ServerResponse_getContent_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, setContent, ServerResponse_setContent_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, addHeaderCallback, ServerResponse_addHeaderCallback_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(ServerResponse, setHeaderCallbacks, ServerResponse_setHeaderCallbacks_args, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
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

zend_class_entry *ServerResponseSender_ce_ptr;

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
