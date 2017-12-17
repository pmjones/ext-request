
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>
#include <string.h>

#include "main/php.h"
#include "main/php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"

#include "php_request.h"

extern PHP_MINIT_FUNCTION(serverrequest);
extern PHP_MINFO_FUNCTION(serverresponse);
extern PHP_MSHUTDOWN_FUNCTION(serverrequest);

/* {{{ server_request_normalize_header_name */
void server_request_normalize_header_name(char *key, size_t key_length)
{
    register char *r = key;
    register char *r_end = r + key_length - 1;

    *r = tolower((unsigned char) *r);
    r++;
    for( ; r <= r_end; r++ ) {
        if( *r == '_' ) {
            *r = '-';
        } else {
            *r = tolower((unsigned char) *r);
        }
    }
}

zend_string *server_request_normalize_header_name_ex(zend_string *in)
{
    zend_string * out = php_trim(in, ZEND_STRL(" \t\r\n\v"), 3);
    server_request_normalize_header_name(ZSTR_VAL(out), ZSTR_LEN(out));
    zend_string_forget_hash_val(out);
    zend_string_hash_val(out);
    return out;
}
/* }}} server_request_normalize_header_name */

/* {{{ PHP_MINIT_FUNCTION */
static PHP_MINIT_FUNCTION(request)
{
    PHP_MINIT(serverrequest)(INIT_FUNC_ARGS_PASSTHRU);
    PHP_MINIT(serverresponse)(INIT_FUNC_ARGS_PASSTHRU);

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION */
static PHP_MINFO_FUNCTION(request)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "Version", PHP_REQUEST_VERSION);
    php_info_print_table_end();
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION */
static PHP_MSHUTDOWN_FUNCTION(request)
{
    PHP_MSHUTDOWN(serverrequest)(SHUTDOWN_FUNC_ARGS_PASSTHRU);
    return SUCCESS;
}
/* }}} */

/* {{{ request_deps */
static const zend_module_dep request_deps[] = {
    ZEND_MOD_REQUIRED("spl")
    ZEND_MOD_REQUIRED("date")
    ZEND_MOD_OPTIONAL("json")
    ZEND_MOD_END
};
/* }}} */

zend_module_entry request_module_entry = {
    STANDARD_MODULE_HEADER_EX, NULL,
    request_deps,                       /* Deps */
    PHP_REQUEST_NAME,                   /* Name */
    NULL,                               /* Functions */
    PHP_MINIT(request),                 /* MINIT */
    PHP_MSHUTDOWN(request),             /* MSHUTDOWN */
    NULL,                               /* RINIT */
    NULL,                               /* RSHUTDOWN */
    PHP_MINFO(request),                 /* MINFO */
    PHP_REQUEST_VERSION,                /* Version */
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_REQUEST
ZEND_GET_MODULE(request)      // Common for all PHP extensions which are build as shared modules
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: et sw=4 ts=4
 */
