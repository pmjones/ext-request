
#ifndef PHP_REQUEST_H
#define PHP_REQUEST_H

#define PHP_REQUEST_NAME "request"
#define PHP_REQUEST_VERSION "1.0.0b1"

extern zend_module_entry request_module_entry;
#define phpext_request_ptr &request_module_entry

#if PHP_VERSION_ID >= 70200

#define REQUEST_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX   ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX
#define REQUEST_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX    ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX

#else

#define REQUEST_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(name, return_reference, required_num_args, type,       allow_null) \
           ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(name, return_reference, required_num_args, type, NULL, allow_null)

#define REQUEST_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(name, return_reference, required_num_args,            classname, allow_null) \
          ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(name, return_reference, required_num_args, IS_OBJECT, classname, allow_null)

#endif

#endif	/* PHP_REQUEST_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: et sw=4 ts=4
 */
