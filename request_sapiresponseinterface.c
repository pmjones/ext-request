
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "main/php.h"

#include "php_request.h"
#include "request_utils.h"
#include "request_sapiresponseinterface.h"

PHP_REQUEST_API zend_class_entry *SapiResponseInterface_ce_ptr;

/* {{{ SapiResponseInterface methods */
static zend_function_entry SapiResponseInterface_methods[] = {
    PHP_ABSTRACT_ME(SapiResponse, setVersion, SapiResponseInterface_setVersion_args)
    PHP_ABSTRACT_ME(SapiResponse, getVersion, SapiResponseInterface_getVersion_args)
    PHP_ABSTRACT_ME(SapiResponse, setCode, SapiResponseInterface_setCode_args)
    PHP_ABSTRACT_ME(SapiResponse, getCode, SapiResponseInterface_getCode_args)
    PHP_ABSTRACT_ME(SapiResponse, addHeader, SapiResponseInterface_addHeader_args)
    PHP_ABSTRACT_ME(SapiResponse, setHeader, SapiResponseInterface_setHeader_args)
    PHP_ABSTRACT_ME(SapiResponse, unsetHeader, SapiResponseInterface_unsetHeader_args)
    PHP_ABSTRACT_ME(SapiResponse, getHeader, SapiResponseInterface_getHeader_args)
    PHP_ABSTRACT_ME(SapiResponse, hasHeader, SapiResponseInterface_hasHeader_args)
    PHP_ABSTRACT_ME(SapiResponse, unsetHeaders, SapiResponseInterface_unsetHeaders_args)
    PHP_ABSTRACT_ME(SapiResponse, getHeaders, SapiResponseInterface_getHeaders_args)
    PHP_ABSTRACT_ME(SapiResponse, getCookie, SapiResponseInterface_getCookie_args)
    PHP_ABSTRACT_ME(SapiResponse, hasCookie, SapiResponseInterface_hasCookie_args)
    PHP_ABSTRACT_ME(SapiResponse, setCookie, SapiResponseInterface_setCookie_args)
    PHP_ABSTRACT_ME(SapiResponse, setRawCookie, SapiResponseInterface_setCookie_args)
    PHP_ABSTRACT_ME(SapiResponse, unsetCookie, SapiResponseInterface_unsetCookie_args)
    PHP_ABSTRACT_ME(SapiResponse, unsetCookies, SapiResponseInterface_unsetCookies_args)
    PHP_ABSTRACT_ME(SapiResponse, getCookies, SapiResponseInterface_getCookies_args)
    PHP_ABSTRACT_ME(SapiResponse, setContent, SapiResponseInterface_setContent_args)
    PHP_ABSTRACT_ME(SapiResponse, getContent, SapiResponseInterface_getContent_args)
    PHP_ABSTRACT_ME(SapiResponse, setHeaderCallbacks, SapiResponseInterface_setHeaderCallbacks_args)
    PHP_ABSTRACT_ME(SapiResponse, addHeaderCallback, SapiResponseInterface_addHeaderCallback_args)
    PHP_ABSTRACT_ME(SapiResponse, getHeaderCallbacks, SapiResponseInterface_getHeaderCallbacks_args)
    PHP_FE_END
};
/* }}} SapiResponseInterface methods */

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(sapiresponseinterface)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "SapiResponseInterface", SapiResponseInterface_methods);
    SapiResponseInterface_ce_ptr = zend_register_internal_interface(&ce);

    return SUCCESS;
}
