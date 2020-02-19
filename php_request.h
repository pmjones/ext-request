
#ifndef PHP_REQUEST_H
#define PHP_REQUEST_H

#define PHP_REQUEST_NAME "request"
#define PHP_REQUEST_VERSION "2.0.0"

#ifdef PHP_WIN32
#	ifdef REQUEST_EXPORTS
#		define PHP_REQUEST_API __declspec(dllexport)
#	elif defined(COMPILE_DL_REQUEST)
#		define PHP_REQUEST_API __declspec(dllimport)
#	else
#		define PHP_REQUEST_API /* nothing */
#	endif
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_REQUEST_API __attribute__ ((visibility("default")))
#else
#	define PHP_REQUEST_API
#endif

extern zend_module_entry request_module_entry;
#define phpext_request_ptr &request_module_entry

extern PHP_REQUEST_API zend_class_entry *ServerRequest_ce_ptr;
extern PHP_REQUEST_API zend_class_entry *ServerResponse_ce_ptr;
extern PHP_REQUEST_API zend_class_entry *ServerResponseInterface_ce_ptr;
extern PHP_REQUEST_API zend_class_entry *ServerResponseSender_ce_ptr;

/* {{{ ServerResponse Argument Info */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponseInterface_getVersion_args, 0, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponseInterface_setVersion_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, version, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponseInterface_getCode_args, 0, 0, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponseInterface_setCode_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, code, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponseInterface_getHeaders_args, 0, 0, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponseInterface_setHeader_args, 0, 2, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, label, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponseInterface_addHeader_args, 0, 2, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, label, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponseInterface_unsetHeader_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, label, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponseInterface_unsetHeaders_args, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponseInterface_getCookies_args, 0, 0, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponseInterface_setCookie_args, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
    ZEND_ARG_INFO(0, expires_or_options)
    ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, domain, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, secure, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, httponly, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponseInterface_unsetCookie_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponseInterface_unsetCookies_args, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ServerResponseInterface_getContent_args, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponseInterface_setContent_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, content)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponseInterface_addHeaderCallback_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponseInterface_setHeaderCallbacks_args, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, callbacks, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ServerResponseInterface_getHeaderCallbacks_args, 0, 0, IS_ARRAY, 1)
ZEND_END_ARG_INFO()
/* }}} Argument Info */

#endif	/* PHP_REQUEST_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: et sw=4 ts=4
 */
