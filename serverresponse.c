
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "main/php.h"
#include "main/php_output.h"
#include "main/php_streams.h"
#include "main/SAPI.h"
#include "main/snprintf.h"
#include "ext/spl/spl_exceptions.h"
#include "ext/date/php_date.h"
#include "ext/standard/head.h"
#include "ext/standard/php_string.h"
#include "ext/standard/url.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_interfaces.h"
#include "Zend/zend_types.h"
#include "Zend/zend_smart_str.h"

#include "php_request.h"

extern zend_string *server_request_normalize_header_name_ex(zend_string *in);

zend_class_entry * ServerResponse_ce_ptr;

/* {{{ Argument Info */
#define AI(method) ServerResponse_##method##_args

ZEND_BEGIN_ARG_INFO_EX(AI(__construct), 0, 0, 0)
ZEND_END_ARG_INFO()

REQUEST_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(AI(getVersion), 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(AI(setVersion), 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, version, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(AI(getStatus), 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(AI(setStatus), 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, status, IS_LONG, 0)
ZEND_END_ARG_INFO()

REQUEST_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(AI(getHeader), 0, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

REQUEST_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(AI(getHeaders), 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(AI(addSetHeader), 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, label, IS_STRING, 0)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

REQUEST_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(AI(getCookies), 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(AI(setCookie), 0, 0, 1)
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

ZEND_BEGIN_ARG_INFO_EX(AI(setContent), 0, 0, 1)
    ZEND_ARG_INFO(0, content)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(AI(setContentJson), 0, 0, 1)
    ZEND_ARG_INFO(0, content)
    ZEND_ARG_TYPE_INFO(0, options, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, depth, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(AI(setContentDownload), 0, 0, 2)
    ZEND_ARG_INFO(0, fh)
    ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, disposition, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, params, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(AI(addHeaderCallback), 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(AI(setHeaderCallbacks), 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, callbacks, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(AI(getHeaderCallbacks), 0, 0, 0)
ZEND_END_ARG_INFO()

REQUEST_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(AI(date), 0, 1, IS_STRING, 0)
    ZEND_ARG_INFO(0, date)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(AI(send), 0, 0, 0)
ZEND_END_ARG_INFO()
/* }}} Argument Info */

/* {{{ Array to CSV */
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
    zval *prev_header = NULL;
    smart_str buf = {0};

    // Read property pointer
    if( !Z_OBJ_HT_P(object)->get_property_ptr_ptr ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "ServerResponse::setHeader requires get_property_ptr_ptr");
        return;
    }

    ZVAL_STRING(&member, "headers");
    prop_ptr = Z_OBJ_HT_P(object)->get_property_ptr_ptr(object, &member, BP_VAR_RW, NULL);
    if( !prop_ptr || Z_TYPE_P(prop_ptr) != IS_ARRAY ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "ServerResponse::$headers must be an array");
        return;
    }

    // Normalize label
    normal_label = server_request_normalize_header_name_ex(label);

    if( !ZSTR_LEN(normal_label) ) {
        zend_string_release(normal_label);
        return;
    }

    // Get previous value
    if( !replace ) {
        prev_header = zend_hash_find(Z_ARRVAL_P(prop_ptr), normal_label);
        if( prev_header ) {
            smart_str_appendz(&buf, prev_header);
            smart_str_appendl_ex(&buf, ZEND_STRL(", "), 0);
        }
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
        smart_str_free(&buf);
    } else {
        // Set/append value
        smart_str_append_ex(&buf, value_str, 0);
        smart_str_0(&buf);
        add_assoc_str_ex(prop_ptr, ZSTR_VAL(normal_label), ZSTR_LEN(normal_label), buf.s);
    }

    zend_string_release(value_str);
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

/* {{{ proto ServerResponse::__construct() */
PHP_METHOD(ServerResponse, __construct)
{
    zval *_this_zval = getThis();
    zval arr;

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    array_init(&arr);
    zend_update_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("headers"), &arr);
    array_init(&arr);
    zend_update_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("cookies"), &arr);
    array_init(&arr);
    zend_update_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("callbacks"), &arr);
}
/* }}} ServerResponse::getVersion */

/* {{{ proto string ServerResponse::getVersion() */
PHP_METHOD(ServerResponse, getVersion)
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
/* }}} ServerResponse::getVersion */

/* {{{ proto void ServerResponse::setVersion(string $version) */
PHP_METHOD(ServerResponse, setVersion)
{
    zval *_this_zval = getThis();
    zend_string *version;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(version)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property_str(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("version"), version);
}
/* }}} ServerResponse::setVersion */

/* {{{ proto int ServerResponse::getStatus() */
PHP_METHOD(ServerResponse, getStatus)
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
/* }}} ServerResponse::getStatus */

/* {{{ proto void ServerResponse::setStatus(int $version) */
PHP_METHOD(ServerResponse, setStatus)
{
    zval *_this_zval = getThis();
    zend_long status;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(status)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property_long(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("status"), status);
}
/* }}} ServerResponse::setStatus */

/* {{{ proto string ServerResponse::getHeader() */
PHP_METHOD(ServerResponse, getHeader)
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

    normal_label = server_request_normalize_header_name_ex(label);

    retval = zend_hash_find(Z_ARRVAL_P(headers), normal_label);
    if( retval ) {
        RETVAL_ZVAL(retval, 1, 0);
    }

    zend_string_release(normal_label);
}
/* }}} ServerResponse::getHeader */

/* {{{ proto array ServerResponse::getHeaders() */
PHP_METHOD(ServerResponse, getHeaders)
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
/* }}} ServerResponse::getHeaders */

/* {{{ proto void ServerResponse::setHeader(string $label, mixed $value) */
PHP_METHOD(ServerResponse, setHeader)
{
    zend_string *label;
    zval *value;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(label)
        Z_PARAM_ZVAL(value)
    ZEND_PARSE_PARAMETERS_END();

    php_response_header(getThis(), label, value, 1);
}
/* }}} ServerResponse::setHeader */

/* {{{ proto void ServerResponse::addHeader(string $label, mixed $value) */
PHP_METHOD(ServerResponse, addHeader)
{
    zend_string *label;
    zval *value;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(label)
        Z_PARAM_ZVAL(value)
    ZEND_PARSE_PARAMETERS_END();

    php_response_header(getThis(), label, value, 0);
}
/* }}} ServerResponse::addHeader */

/* {{{ proto array ServerResponse::getCookies() */
PHP_METHOD(ServerResponse, getCookies)
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
/* }}} ServerResponse::getCookies */

/* {{{ proto void ServerResponse::setCookie(string name [, string value [, int expires [, string path [, string domain [, bool secure[, bool httponly]]]]]]) */
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
    zval member = {0};
    zval cookie = {0};

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
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "ServerResponse::setCookie requires get_property_ptr_ptr");
        return;
    }

    // Read property pointer
    ZVAL_STRING(&member, "cookies");
    ptr = Z_OBJ_HT_P(_this_zval)->get_property_ptr_ptr(_this_zval, &member, BP_VAR_RW, NULL);

    if( !ptr ) {
        // fall-through
    } else if( Z_TYPE_P(ptr) != IS_ARRAY ) {
        convert_to_array(ptr);
        //ptr = NULL;
    }

    // Make cookies array
    array_init_size(&cookie, 7);
    add_assoc_bool_ex(&cookie, ZEND_STRL("raw"), raw);
    if( value ) {
        add_assoc_stringl_ex(&cookie, ZEND_STRL("value"), ZSTR_VAL(value), ZSTR_LEN(value));
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
    zval_ptr_dtor(&member);
}

PHP_METHOD(ServerResponse, setCookie)
{
    php_response_setcookie(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} ServerResponse::setCookie */

/* {{{ proto void ServerResponse::setRawCookie(string name [, string value [, int expires [, string path [, string domain [, bool secure[, bool httponly]]]]]]) */
PHP_METHOD(ServerResponse, setRawCookie)
{
    php_response_setcookie(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} ServerResponse::setRawCookie */

/* {{{ proto mixed ServerResponse::getContent() */
PHP_METHOD(ServerResponse, getContent)
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
/* }}} ServerResponse::getContent */

/* {{{ proto void ServerResponse::setContent(mixed $content) */
PHP_METHOD(ServerResponse, setContent)
{
    zval *_this_zval = getThis();
    zval *content;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(content)
    ZEND_PARSE_PARAMETERS_END();

    zend_update_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("content"), content);
}
/* }}} ServerResponse::setContent */

/* {{{ proto void ServerResponse::setContentJson(mixed $value [, int $options [, int $depth = 512]]) */
static inline void throw_json_exception()
{
    zval func_name = {0};
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

PHP_METHOD(ServerResponse, setContentJson)
{
    zval *value;
    zend_long options = 0;
    zend_long depth = 512;

    zval *_this_zval = getThis();
    zval func_name = {0};
    zval json_value = {0};
    zval params[3] = {0};
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
    php_response_header_stringl(_this_zval, ZEND_STRL("content-type"), ZEND_STRL("application/json"), 1);
}
/* }}} ServerResponse::setContentJson */

/* {{{ proto void ServerResponse::setContentDownload(resource $fh, string $name [, string $disposition = 'attachment' [, array $params = array()]]) */
PHP_METHOD(ServerResponse, setContentDownload)
{
    zval *zstream;
    zend_string *name = NULL;
    zend_string *disposition = NULL;
    zval *params = NULL;
    zval _params = {0};
    zval *_this_zval = getThis();
    zend_string *tmp_filename;
    smart_str buf = {0};
    smart_str buf2 = {0};

    ZEND_PARSE_PARAMETERS_START(2, 4)
        Z_PARAM_ZVAL(zstream)
        Z_PARAM_STR(name)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(disposition)
        Z_PARAM_ARRAY(params)
    ZEND_PARSE_PARAMETERS_END();

    // Check type of resource parameter
    if( Z_TYPE_P(zstream) != IS_RESOURCE ) {
        zend_throw_exception_ex(spl_ce_InvalidArgumentException, 0, "Argument 1 passed to ServerResponse::setContentDownload() must be of the type resource, string given");
        return;
    }

    // Initialize params
    if( !params || Z_TYPE_P(params) != IS_ARRAY ) {
        params = &_params;
        array_init(params);
    }

    // Set headers
    php_response_header_stringl(_this_zval, ZEND_STRL("content-type"), ZEND_STRL("application/octet-stream"), 1);
    php_response_header_stringl(_this_zval, ZEND_STRL("content-transfer-encoding"), ZEND_STRL("binary"), 1);

    // Set name
    tmp_filename = php_raw_url_encode(ZSTR_VAL(name), ZSTR_LEN(name));
    smart_str_appendc(&buf, '"');
    smart_str_append(&buf, tmp_filename);
    smart_str_appendc(&buf, '"');
    smart_str_0(&buf);
    zend_string_release(tmp_filename);
    add_assoc_str_ex(params, ZEND_STRL("filename"), buf.s);

    // Set disposition
    if( disposition ) {
        smart_str_append(&buf2, disposition);
    } else {
        smart_str_appendl_ex(&buf2, ZEND_STRL("attachment"), 0);
    }
    smart_str_appendc(&buf2, ';');
    _array_to_semicsv(&buf2, params);
    smart_str_0(&buf2);
    php_response_header_stringl(_this_zval, ZEND_STRL("content-disposition"), ZSTR_VAL(buf2.s), ZSTR_LEN(buf2.s), 1);
    smart_str_free(&buf2);

    // Update content
    zend_update_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("content"), zstream);

    zval_dtor(&_params);
}
/* }}} ServerResponse::setContentDownload */

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
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "ServerResponse::$callbacks must be an array");
        goto done;
    }

    // Append callback
    zval tmp = {0};
    ZVAL_ZVAL(&tmp, callback_func, 1, 0);
    add_next_index_zval(prop_ptr, &tmp);

    // Cleanup
done:
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
    zend_update_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("callbacks"), &arr);

    // Forward each item to addHeaderCallback
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(callbacks), callback) {
        zend_call_method_with_1_params(_this_zval, NULL, NULL, "addHeaderCallback", NULL, callback);
    } ZEND_HASH_FOREACH_END();
}
/* }}} ServerResponse::setHeaderCallbacks */

/* {{{ proto callback ServerResponse::getHeaderCallbacks() */
PHP_METHOD(ServerResponse, getHeaderCallbacks)
{
    zval *_this_zval = getThis();
    zval *retval;

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    retval = zend_read_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("callbacks"), 0, NULL);
    if( retval ) {
        RETVAL_ZVAL(retval, 1, 0);
    }
}
/* }}} ServerResponse::getHeaderCallback */

/* {{{ proto string ServerResponse::date(mixed $date) */
PHP_METHOD(ServerResponse, date)
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
/* }}} ServerResponse::date */

/* {{{ proto void ServerResponse::send() */
PHP_METHOD(ServerResponse, send)
{
    zval *_this_zval = getThis();
    zval rv = {0};

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    zend_call_method_with_0_params(_this_zval, NULL, NULL, "runHeaderCallbacks", &rv);
    zend_call_method_with_0_params(_this_zval, NULL, NULL, "sendStatus", &rv);
    zend_call_method_with_0_params(_this_zval, NULL, NULL, "sendHeaders", &rv);
    zend_call_method_with_0_params(_this_zval, NULL, NULL, "sendCookies", &rv);
    zend_call_method_with_0_params(_this_zval, NULL, NULL, "sendContent", &rv);

    zval_ptr_dtor(&rv);
}
/* }}} ServerResponse::send */

/* {{{ proto void ServerResponse::runHeaderCallbacks() */
PHP_METHOD(ServerResponse, runHeaderCallbacks)
{
    zval *_this_zval = getThis();
    zval *callbacks;
    zval *callback;

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    // Make status
    callbacks = zend_read_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("callbacks"), 0, NULL);

    if( !callbacks || Z_TYPE_P(callbacks) != IS_ARRAY ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "ServerResponse::$callbacks must be an array");
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
            ZVAL_ZVAL(&params[0], _this_zval, 1, 0);
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
            php_error_docref(NULL, E_WARNING, "Could not call the header callback");
        }

        if (callback_error) {
            efree(callback_error);
        }
    } ZEND_HASH_FOREACH_END();
}
/* }}} ServerResponse::runHeaderCallbacks */

/* {{{ proto void ServerResponse::sendStatus() */
PHP_METHOD(ServerResponse, sendStatus)
{
    zval *_this_zval = getThis();
    sapi_header_line ctr = {0};
    zval *tmp;
    smart_str buf = {0};

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    // Make status
    tmp = zend_read_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("status"), 0, NULL);
    if( tmp ) {
        ctr.response_code = zval_get_long(tmp);
    } else {
        ctr.response_code = 200;
    }

    // Make header
    smart_str_appendl_ex(&buf, ZEND_STRL("HTTP/"), 0);

    tmp = zend_read_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("status"), 0, NULL);
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
/* }}} ServerResponse::sendStatus */

/* {{{ proto void ServerResponse::sendHeaders() */
static inline void send_header(zend_string *header, zval *value)
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

PHP_METHOD(ServerResponse, sendHeaders)
{
    zval *_this_zval = getThis();
    zval *tmp;
    zend_string *key;
    zend_ulong index;
    zval *val;

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    tmp = zend_read_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("headers"), 0, NULL);

    if( !tmp || Z_TYPE_P(tmp) != IS_ARRAY ) {
        return;
    }

    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(tmp), index, key, val) {
        if( key ) {
            send_header(key, val);
        }
    } ZEND_HASH_FOREACH_END();
}
/* }}} ServerResponse::sendHeaders */

/* {{{ proto void ServerResponse::sendCookies() */
static inline void send_cookie(zend_string *name, zval *arr)
{
    zval *tmp;
    zend_string *value;
    zend_long expires = 0;
    zend_string *path = NULL;
    zend_string *domain = NULL;
    zend_bool secure = 0;
    zend_bool httponly = 0;
    zend_bool raw = 0;

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(arr), ZEND_STRL("value"))) ) {
        value = zval_get_string(tmp);
    } else {
        return;
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(arr), ZEND_STRL("expire"))) ) {
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

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(arr), ZEND_STRL("raw"))) ) {
        raw = zval_is_true(tmp);
    }

    php_setcookie(name, value, expires, path, domain, secure, !raw, httponly);
}

PHP_METHOD(ServerResponse, sendCookies)
{
    zval *_this_zval = getThis();
    zval *tmp;
    zend_string *key;
    zend_ulong index;
    zval *val;

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    tmp = zend_read_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("cookies"), 0, NULL);

    if( !tmp || Z_TYPE_P(tmp) != IS_ARRAY ) {
        return;
    }

    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(tmp), index, key, val) {
        if( key && Z_TYPE_P(val) == IS_ARRAY ) {
            send_cookie(key, val);
        }
    } ZEND_HASH_FOREACH_END();
}
/* }}} ServerResponse::sendCookies */

/* {{{ proto void ServerResponse::sendContent() */
PHP_METHOD(ServerResponse, sendContent)
{
    zval *_this_zval = getThis();
    zval *tmp;
    zend_string *tmp_str;
    php_stream *stream;
    char *error;
    zval func_name = {0};
    zval rv = {0};
    zval params[1] = {0};

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    tmp = zend_read_property(Z_OBJCE_P(_this_zval), _this_zval, ZEND_STRL("content"), 0, NULL);

    if( !tmp ) {
        return;
    }

    if( Z_TYPE_P(tmp) == IS_OBJECT && zend_is_callable(tmp, 0, NULL) ) {
        ZVAL_STRING(&func_name, "__invoke");
        ZVAL_ZVAL(&params[0], _this_zval, 1, 0);
        call_user_function(&Z_OBJCE_P(tmp)->function_table, tmp, &func_name, &rv, 1, &params);
        zval_ptr_dtor(&func_name);
        zval_ptr_dtor(&params[0]);
        tmp = &rv;
    } else {
        Z_TRY_ADDREF_P(tmp);
    }

    switch( Z_TYPE_P(tmp) ) {
        case IS_UNDEF:
        case IS_NULL:
        case IS_FALSE:
            // do nothing
            break;

        case IS_RESOURCE:
            php_stream_from_res(stream, Z_RES_P(tmp)); // this macro can return
            php_stream_seek(stream, 0, SEEK_SET);
            php_stream_passthru(stream);
            break;

        case IS_STRING:
            php_output_write(Z_STRVAL_P(tmp), Z_STRLEN_P(tmp));
            break;

        default:
            tmp_str = zval_get_string(tmp);
            php_output_write(ZSTR_VAL(tmp_str), ZSTR_LEN(tmp_str));
            zend_string_release(tmp_str);
            break;
    }

    zval_ptr_dtor(tmp);
}
/* }}} ServerResponse::sendContent */

/* {{{ ServerResponse methods */
static zend_function_entry ServerResponse_methods[] = {
    PHP_ME(ServerResponse, __construct, AI(__construct), ZEND_ACC_PUBLIC)
    PHP_ME(ServerResponse, getVersion, AI(getVersion), ZEND_ACC_PUBLIC)
    PHP_ME(ServerResponse, setVersion, AI(setVersion), ZEND_ACC_PUBLIC)
    PHP_ME(ServerResponse, getStatus, AI(getStatus), ZEND_ACC_PUBLIC)
    PHP_ME(ServerResponse, setStatus, AI(setStatus), ZEND_ACC_PUBLIC)
    PHP_ME(ServerResponse, getHeader, AI(getHeader), ZEND_ACC_PUBLIC)
    PHP_ME(ServerResponse, getHeaders, AI(getHeaders), ZEND_ACC_PUBLIC)
    PHP_ME(ServerResponse, setHeader, AI(addSetHeader), ZEND_ACC_PUBLIC)
    PHP_ME(ServerResponse, addHeader, AI(addSetHeader), ZEND_ACC_PUBLIC)
    PHP_ME(ServerResponse, getCookies, AI(getCookies), ZEND_ACC_PUBLIC)
    PHP_ME(ServerResponse, setCookie, AI(setCookie), ZEND_ACC_PUBLIC)
    PHP_ME(ServerResponse, setRawCookie, AI(setCookie), ZEND_ACC_PUBLIC)
    PHP_ME(ServerResponse, getContent, AI(getContent), ZEND_ACC_PUBLIC)
    PHP_ME(ServerResponse, setContent, AI(setContent), ZEND_ACC_PUBLIC)
    PHP_ME(ServerResponse, setContentJson, AI(setContentJson), ZEND_ACC_PUBLIC)
    PHP_ME(ServerResponse, setContentDownload, AI(setContentDownload), ZEND_ACC_PUBLIC)
    PHP_ME(ServerResponse, addHeaderCallback, AI(addHeaderCallback), ZEND_ACC_PUBLIC)
    PHP_ME(ServerResponse, setHeaderCallbacks, AI(setHeaderCallbacks), ZEND_ACC_PUBLIC)
    PHP_ME(ServerResponse, getHeaderCallbacks, AI(getHeaderCallbacks), ZEND_ACC_PUBLIC)
    PHP_ME(ServerResponse, date, AI(date), ZEND_ACC_PUBLIC)
    PHP_ME(ServerResponse, send, AI(send), ZEND_ACC_PUBLIC)
    PHP_ME(ServerResponse, runHeaderCallbacks, AI(send), ZEND_ACC_PROTECTED)
    PHP_ME(ServerResponse, sendStatus, AI(send), ZEND_ACC_PROTECTED)
    PHP_ME(ServerResponse, sendHeaders, AI(send), ZEND_ACC_PROTECTED)
    PHP_ME(ServerResponse, sendCookies, AI(send), ZEND_ACC_PROTECTED)
    PHP_ME(ServerResponse, sendContent, AI(send), ZEND_ACC_PROTECTED)
    PHP_FE_END
};
/* }}} ServerResponse methods */

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(serverresponse)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "ServerResponse", ServerResponse_methods);
    ServerResponse_ce_ptr = zend_register_internal_class(&ce);

    zend_declare_property_stringl(ServerResponse_ce_ptr, ZEND_STRL("version"), ZEND_STRL("1.1"), ZEND_ACC_PROTECTED);
    zend_declare_property_long(ServerResponse_ce_ptr, ZEND_STRL("status"), 200, ZEND_ACC_PROTECTED);
    zend_declare_property_null(ServerResponse_ce_ptr, ZEND_STRL("headers"), ZEND_ACC_PROTECTED);
    zend_declare_property_null(ServerResponse_ce_ptr, ZEND_STRL("cookies"), ZEND_ACC_PROTECTED);
    zend_declare_property_null(ServerResponse_ce_ptr, ZEND_STRL("content"), ZEND_ACC_PROTECTED);
    zend_declare_property_null(ServerResponse_ce_ptr, ZEND_STRL("callbacks"), ZEND_ACC_PROTECTED);

    return SUCCESS;
}
/* }}} PHP_MINIT_FUNCTION */
