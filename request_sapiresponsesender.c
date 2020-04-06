
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "main/php.h"
#include "main/SAPI.h"

#include "Zend/zend_exceptions.h"
#include "Zend/zend_interfaces.h"

#include "php_request.h"
#include "request_utils.h"



PHP_REQUEST_API zend_class_entry *SapiResponseSender_ce_ptr;

/* {{{ Argument Info */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(SapiResponseSender_send_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, response, SapiResponseInterface, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(SapiResponseSender_runHeaderCallbacks_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, response, SapiResponseInterface, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(SapiResponseSender_sendStatus_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, response, SapiResponseInterface, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(SapiResponseSender_sendHeaders_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, response, SapiResponseInterface, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(SapiResponseSender_sendCookies_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, response, SapiResponseInterface, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(SapiResponseSender_sendContent_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, response, SapiResponseInterface, 0)
ZEND_END_ARG_INFO()
/* }}} Argument Info */

/* {{{ proto void SapiResponseSender::runHeaderCallbacks() */
static void sapi_response_sender_run_header_callbacks(zval *response)
{
    zval callbacks = {0};
    zval *callback;

#if PHP_MAJOR_VERSION >= 8
    zend_call_method_with_0_params(Z_OBJ_P(response), Z_OBJCE_P(response), NULL, "getheadercallbacks", &callbacks);
#else
    zend_call_method_with_0_params(response, Z_OBJCE_P(response), NULL, "getheadercallbacks", &callbacks);
#endif

    if( Z_TYPE(callbacks) != IS_ARRAY ) {
        zval_ptr_dtor(&callbacks);
        return;
    }

    ZEND_HASH_FOREACH_VAL(Z_ARRVAL(callbacks), callback) {

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

    zval_ptr_dtor(&callbacks);
}

PHP_METHOD(SapiResponseSender, runHeaderCallbacks)
{
    zval *response;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(response, SapiResponseInterface_ce_ptr)
    ZEND_PARSE_PARAMETERS_END();

    sapi_response_sender_run_header_callbacks(response);
}
/* }}} SapiResponseSender::runHeaderCallbacks */

/* {{{ proto void SapiResponseSender::sendStatus() */
static void sapi_response_sender_send_status(zval *response)
{
    sapi_header_line ctr = {0};
    zval code = {0};
    zval version = {0};
    smart_str buf = {0};

#if PHP_MAJOR_VERSION >= 8
    zend_call_method_with_0_params(Z_OBJ_P(response), Z_OBJCE_P(response), NULL, "getcode", &code);
    zend_call_method_with_0_params(Z_OBJ_P(response), Z_OBJCE_P(response), NULL, "getversion", &version);
#else
    zend_call_method_with_0_params(response, Z_OBJCE_P(response), NULL, "getcode", &code);
    zend_call_method_with_0_params(response, Z_OBJCE_P(response), NULL, "getversion", &version);
#endif

    // Make code
    if( Z_TYPE(code) == IS_LONG ) {
        ctr.response_code = zval_get_long(&code);
    } else {
        ctr.response_code = 200;
    }

    // Make status
    smart_str_appendl_ex(&buf, ZEND_STRL("HTTP/"), 0);

    if (Z_TYPE(version) == IS_NULL || Z_TYPE(version) == IS_UNDEF) {
        smart_str_appendl_ex(&buf, ZEND_STRL("1.1"), 0);
    } else {
        convert_to_string(&version);
        smart_str_append_ex(&buf, Z_STR(version), 0);
    }

    smart_str_appendc_ex(&buf, ' ', 0);
    smart_str_append_long_ex(&buf, ctr.response_code, 0);
    smart_str_0(&buf);

    ctr.line = ZSTR_VAL(buf.s);
    ctr.line_len = ZSTR_LEN(buf.s);

    sapi_header_op(SAPI_HEADER_REPLACE, &ctr);

    smart_str_free(&buf);

    zval_ptr_dtor(&code);
    zval_ptr_dtor(&version);
}

PHP_METHOD(SapiResponseSender, sendStatus)
{
    zval *response;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(response, SapiResponseInterface_ce_ptr)
    ZEND_PARSE_PARAMETERS_END();

    sapi_response_sender_send_status(response);
}
/* }}} SapiResponseSender::sendStatus */

/* {{{ proto void SapiResponseSender::sendHeaders() */
static inline void sapi_response_sender_send_header(zend_string *header, zval *value)
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

static void sapi_response_sender_send_headers(zval *response)
{
    zval headers = {0};
    zend_ulong index;
    zend_string *label;
    zval *value;

#if PHP_MAJOR_VERSION >= 8
    zend_call_method_with_0_params(Z_OBJ_P(response), Z_OBJCE_P(response), NULL, "getheaders", &headers);
#else
    zend_call_method_with_0_params(response, Z_OBJCE_P(response), NULL, "getheaders", &headers);
#endif

    if( Z_TYPE(headers) == IS_ARRAY ) {
        ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(headers), index, label, value) {
            if( label ) {
                sapi_response_sender_send_header(label, value);
            }
        } ZEND_HASH_FOREACH_END();
    }

    zval_ptr_dtor(&headers);
}

PHP_METHOD(SapiResponseSender, sendHeaders)
{
    zval *response;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(response, SapiResponseInterface_ce_ptr)
    ZEND_PARSE_PARAMETERS_END();

    sapi_response_sender_send_headers(response);
}
/* }}} SapiResponseSender::sendHeaders */

/* {{{ proto void SapiResponseSender::sendCookies() */
static inline void sapi_response_sender_send_cookie(zend_string *name, zval *arr)
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

static void sapi_response_sender_send_cookies(zval *response)
{
    zval cookies = {0};
    zend_string *key;
    zend_ulong index;
    zval *val;

#if PHP_MAJOR_VERSION >= 8
    zend_call_method_with_0_params(Z_OBJ_P(response), Z_OBJCE_P(response), NULL, "getcookies", &cookies);
#else
    zend_call_method_with_0_params(response, Z_OBJCE_P(response), NULL, "getcookies", &cookies);
#endif

    if( Z_TYPE(cookies) == IS_ARRAY ) {
        ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(cookies), index, key, val) {
            if( key && Z_TYPE_P(val) == IS_ARRAY ) {
                sapi_response_sender_send_cookie(key, val);
            }
        } ZEND_HASH_FOREACH_END();
    }

    zval_ptr_dtor(&cookies);
}

PHP_METHOD(SapiResponseSender, sendCookies)
{
    zval *response;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(response, SapiResponseInterface_ce_ptr)
    ZEND_PARSE_PARAMETERS_END();

    sapi_response_sender_send_cookies(response);
}
/* }}} SapiResponseSender::sendCookies */

/* {{{ proto void SapiResponseSender::sendContent() */
static inline void sapi_response_sender_send_iterable(zval *content) {
    zend_object_iterator *iter;
    zend_class_entry *ce = Z_OBJCE_P(content);
    zval *val;
    zend_string *val_str;

    iter = ce->get_iterator(ce, content, 0);

    if (EG(exception)) {
        goto done;
    }

    if (iter->funcs->rewind) {
        iter->funcs->rewind(iter);
        if (EG(exception)) {
            goto done;
        }
    }

    while (iter->funcs->valid(iter) == SUCCESS) {
        if (EG(exception)) {
            goto done;
        }

        val = iter->funcs->get_current_data(iter);
        if (EG(exception)) {
            goto done;
        }

        val_str = zval_get_string(val);
        php_output_write(ZSTR_VAL(val_str), ZSTR_LEN(val_str));
        zend_string_release(val_str);
        zval_ptr_dtor(val);

        iter->funcs->move_forward(iter);
        if (EG(exception)) {
            goto done;
        }
    }

done:
    OBJ_RELEASE(&iter->std);
}

static void sapi_response_sender_send_content(zval *response)
{
    zval *content;
    zend_string *content_str;
    php_stream *stream;
    zval func_name = {0};
    zval rv = {0}, rv2 = {0};
    zval params[1] = {0};

    // Call getContent
#if PHP_MAJOR_VERSION >= 8
    zend_call_method_with_0_params(Z_OBJ_P(response), Z_OBJCE_P(response), NULL, "getcontent", &rv2);
#else
    zend_call_method_with_0_params(response, Z_OBJCE_P(response), NULL, "getcontent", &rv2);
#endif
    content = &rv2;

    // Call again if the value was callable
    if( Z_TYPE_P(content) == IS_OBJECT && zend_is_callable(content, 0, NULL) ) {
        ZVAL_STRING(&func_name, "__invoke");
        ZVAL_ZVAL(&params[0], response, 1, 0);
        call_user_function(&Z_OBJCE_P(content)->function_table, content, &func_name, &rv, 1, params);
        zval_ptr_dtor(&func_name);
        zval_ptr_dtor(&params[0]);
        content = &rv;
        zval_ptr_dtor(&rv2);
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
            // ported from php_stream_from_res
            stream = zend_fetch_resource2(Z_RES_P(content), "stream", php_file_le_stream(), php_file_le_pstream());
            if (stream) {
                php_stream_seek(stream, 0, SEEK_SET);
                php_stream_passthru(stream);
            }
            break;

        case IS_STRING:
            php_output_write(Z_STRVAL_P(content), Z_STRLEN_P(content));
            break;

        case IS_OBJECT:
            if (instanceof_function(Z_OBJCE_P(content), zend_ce_traversable)) {
                sapi_response_sender_send_iterable(content);
                break;
            }
            // !fallthrough to default!

        default:
            content_str = zval_get_string(content);
            php_output_write(ZSTR_VAL(content_str), ZSTR_LEN(content_str));
            zend_string_release(content_str);
            break;
    }

    zval_ptr_dtor(content);
}

PHP_METHOD(SapiResponseSender, sendContent)
{
    zval *response;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(response, SapiResponseInterface_ce_ptr)
    ZEND_PARSE_PARAMETERS_END();

    sapi_response_sender_send_content(response);

}
/* }}} SapiResponseSender::sendContent */

/* {{{ proto void SapiResponseSender::send() */
PHP_METHOD(SapiResponseSender, send)
{
    zval *response;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(response, SapiResponseInterface_ce_ptr)
    ZEND_PARSE_PARAMETERS_END();

    sapi_response_sender_run_header_callbacks(response);
    sapi_response_sender_send_status(response);
    sapi_response_sender_send_headers(response);
    sapi_response_sender_send_cookies(response);
    sapi_response_sender_send_content(response);
}
/* }}} SapiResponseSender::send */

/* {{{ SapiResponseSender methods */
static zend_function_entry SapiResponseSender_methods[] = {
    PHP_ME(SapiResponseSender, send, SapiResponseSender_send_args, ZEND_ACC_PUBLIC)
    PHP_ME(SapiResponseSender, runHeaderCallbacks, SapiResponseSender_runHeaderCallbacks_args, ZEND_ACC_PUBLIC)
    PHP_ME(SapiResponseSender, sendStatus, SapiResponseSender_sendStatus_args, ZEND_ACC_PUBLIC)
    PHP_ME(SapiResponseSender, sendHeaders, SapiResponseSender_sendHeaders_args, ZEND_ACC_PUBLIC)
    PHP_ME(SapiResponseSender, sendCookies, SapiResponseSender_sendCookies_args, ZEND_ACC_PUBLIC)
    PHP_ME(SapiResponseSender, sendContent, SapiResponseSender_sendContent_args, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
/* }}} SapiResponseSender methods */

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(sapiresponsesender)
{
    zend_class_entry SapiResponseSender_ce;

    INIT_CLASS_ENTRY(SapiResponseSender_ce, "SapiResponseSender", SapiResponseSender_methods);
    SapiResponseSender_ce_ptr = zend_register_internal_class(&SapiResponseSender_ce);

    return SUCCESS;
}
/* }}} PHP_MINIT_FUNCTION */
