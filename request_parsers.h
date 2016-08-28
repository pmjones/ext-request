
#ifndef REQUEST_SCANNERS_H
#define REQUEST_SCANNERS_H

#include "main/php.h"

PHPAPI void php_request_parse_accept(zval *return_value, const unsigned char *str, size_t len);
PHPAPI void php_request_parse_content_type(zval *return_value, const unsigned char *str, size_t len);
PHPAPI void php_request_parse_digest_auth(zval *return_value, const unsigned char *str, size_t len);

#endif	/* REQUEST_SCANNERS_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: et sw=4 ts=4
 */
