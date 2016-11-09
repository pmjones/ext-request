
#ifndef PHP_REQUEST_H
#define PHP_REQUEST_H

#define PHP_REQUEST_NAME "request"
#define PHP_REQUEST_VERSION "0.0.0"

extern zend_module_entry request_module_entry;
#define phpext_request_ptr &request_module_entry

PHPAPI void server_request_normalize_header_name(char *key, size_t key_length);
PHPAPI zend_string *server_request_normalize_header_name_ex(zend_string *in);

#endif	/* PHP_REQUEST_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: et sw=4 ts=4
 */
