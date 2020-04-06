
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

extern PHP_REQUEST_API zend_class_entry *SapiRequest_ce_ptr;
extern PHP_REQUEST_API zend_class_entry *SapiResponse_ce_ptr;
extern PHP_REQUEST_API zend_class_entry *SapiResponseInterface_ce_ptr;
extern PHP_REQUEST_API zend_class_entry *SapiResponseSender_ce_ptr;
extern PHP_REQUEST_API zend_class_entry *SapiUpload_ce_ptr;

#endif	/* PHP_REQUEST_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: et sw=4 ts=4
 */
