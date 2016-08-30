
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include "main/php.h"
#include "main/php_ini.h"
#include "main/SAPI.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "ext/standard/url.h"
#include "ext/spl/spl_exceptions.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_portability.h"
#include "Zend/zend_smart_str.h"

#include "php_request.h"
#include "request_parsers.h"
#include "request_utils.h"

zend_class_entry * PhpRequest_ce_ptr;
static zend_object_handlers PhpRequest_obj_handlers;
static HashTable PhpRequest_prop_handlers;

struct php_request_obj {
    zend_bool locked;
    zend_object std;
};

/* {{{ Argument Info */
ZEND_BEGIN_ARG_INFO_EX(PhpRequest_construct_args, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(PhpRequest_parseAccept_args, IS_ARRAY, NULL, 1)
    ZEND_ARG_TYPE_INFO(0, header, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(PhpRequest_parseContentType_args, IS_ARRAY, NULL, 1)
    ZEND_ARG_TYPE_INFO(0, header, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(PhpRequest_parseDigestAuth_args, IS_ARRAY, NULL, 1)
    ZEND_ARG_TYPE_INFO(0, header, IS_STRING, 0)
ZEND_END_ARG_INFO()
/* }}} Argument Info */

/* {{{ Z_REQUEST_P */
static inline struct php_request_obj *php_request_fetch_object(zend_object *obj)
{
    return (struct php_request_obj *)((char*)(obj) - XtOffsetOf(struct php_request_obj, std));
}
#define Z_REQUEST_P(zv) php_request_fetch_object(Z_OBJ_P((zv)))
/* }}} */

/* {{{ php_request_obj_create */
static zend_object *php_request_obj_create(zend_class_entry *ce)
{
    struct php_request_obj *obj;

    obj = ecalloc(1, sizeof(*obj) + zend_object_properties_size(ce));
    zend_object_std_init(&obj->std, ce);
    object_properties_init(&obj->std, ce);
    obj->std.handlers = &PhpRequest_obj_handlers;

    obj->locked = 0;

    return &obj->std;
}
/* }}} */

/* {{{ php_request_object_has_property */
static int php_request_object_has_property(zval *object, zval *member, int has_set_exists, void **cache_slot)
{
    if( !std_object_handlers.has_property(object, member, has_set_exists, cache_slot) ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "PhpRequest::$%.*s does not exist.", Z_STRLEN_P(member), Z_STRVAL_P(member));
        return 0;
    } else {
        return 1;
    }
}
/* }}} */

/* {{{ php_request_object_read_property */
static zval *php_request_object_read_property(zval *object, zval *member, int type, void **cache_slot, zval *rv)
{
    zval *retval;
    if( !std_object_handlers.has_property(object, member, 2, cache_slot) ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "PhpRequest::$%.*s does not exist.", Z_STRLEN_P(member), Z_STRVAL_P(member));
        return rv;
    }
    retval = std_object_handlers.read_property(object, member, type, cache_slot, rv);
    // Make sure the property can't be modified
    if( !Z_ISREF_P(rv) && (type == BP_VAR_W || type == BP_VAR_RW  || type == BP_VAR_UNSET) ) {
        SEPARATE_ZVAL(rv);
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "PhpRequest is read-only.");
    }
    return retval;
}
/* }}} */

/* {{{ php_request_object_write_property */
static void php_request_object_write_property(zval *object, zval *member, zval *value, void **cache_slot)
{
    struct php_request_obj *intern = Z_REQUEST_P(object);
    if( intern->locked ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "PhpRequest is read-only.");
    } else {
        std_object_handlers.write_property(object, member, value, cache_slot);
    }
}
/* }}} */

/* {{{ php_request_object_unset_property */
static void php_request_object_unset_property(zval *object, zval *member, void **cache_slot)
{
    struct php_request_obj *intern = Z_REQUEST_P(object);
    if( intern->locked ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "PhpRequest is read-only.");
    } else {
        std_object_handlers.unset_property(object, member, cache_slot);
    }
}
/* }}} */

/* {{{ proto PhpRequest::__construct() */
static inline void copy_global(zval* obj, const char* key, size_t key_len, const char* sg, size_t sg_len)
{
    zval *tmp = zend_hash_str_find(&EG(symbol_table), sg, sg_len);
    if( tmp ) {
        zend_update_property(Z_CE_P(obj), obj, key, key_len, tmp);
        Z_TRY_ADDREF_P(tmp);
    }
}
#define copy_global_lit(obj, glob, key) copy_global(obj, ZEND_STRL(glob), ZEND_STRL(key))

static inline void set_url(zval *object, zval *server)
{
    zend_string *tmp;
    php_url *url;
    zval arr = {0};

    tmp = php_request_detect_url(server);
    if( !tmp ) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, "Could not determine host for PhpRequest.");
        return;
    }

    url = php_url_parse_ex(ZSTR_VAL(tmp), ZSTR_LEN(tmp));
    zend_string_release(tmp);
    if( !url ) {
        return;
    }

    // Form array
    array_init(&arr);
    if( url->scheme ) {
        add_assoc_string(&arr, "scheme", url->scheme);
    } else {
        add_assoc_null(&arr, "scheme");
    }
    if( url->host ) {
        add_assoc_string(&arr, "host", url->host);
    } else {
        add_assoc_null(&arr, "host");
    }
    if( url->port ) {
        add_assoc_long(&arr, "port", url->port);
    } else {
        add_assoc_null(&arr, "port");
    }
    if( url->user ) {
        add_assoc_string(&arr, "user", url->user);
    } else {
        add_assoc_null(&arr, "user");
    }
    if( url->pass ) {
        add_assoc_string(&arr, "pass", url->pass);
    } else {
        add_assoc_null(&arr, "pass");
    }
    if( url->path ) {
        add_assoc_string(&arr, "path", url->path);
    } else {
        add_assoc_null(&arr, "path");
    }
    if( url->query ) {
        add_assoc_string(&arr, "query", url->query);
    } else {
        add_assoc_null(&arr, "query");
    }
    if( url->fragment ) {
        add_assoc_string(&arr, "fragment", url->fragment);
    } else {
        add_assoc_null(&arr, "fragment");
    }

    convert_to_object(&arr);

    zend_update_property(Z_CE_P(object), object, ZEND_STRL("url"), &arr);

    php_url_free(url);
}

static inline void set_accept_by_name(zval *object, zval *server, const char *src, size_t src_length, const char *dest, size_t dest_length)
{
    zval val;
    zval *tmp;

    array_init(&val);
    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), src, src_length)) ) {
        php_request_parse_accept(&val, Z_STRVAL_P(tmp), Z_STRLEN_P(tmp));
    }
    zend_update_property(Z_CE_P(object), object, dest, dest_length, &val);
}

static inline void parse_accept_language(zval *lang)
{
    zend_string *key;
    zend_ulong index;
    zval *val;
    zval *value;
    zval rv;
    zend_string *str;
    char *r1;
    char *r2;
    zval type = {0};
    zval subtype = {0};

    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(lang), val) {
        value = zend_read_property(Z_CE_P(val), val, ZEND_STRL("value"), 0, &rv);
        if( value ) {
            str = zval_get_string(value);
            r1 = ZSTR_VAL(str);
            r2 = memchr(r1, '-', ZSTR_LEN(str));
            if( r2 ) {
                ZVAL_STRINGL(&type, r1, r2 - r1);
                ZVAL_STRINGL(&subtype, r2 + 1, r1 + ZSTR_LEN(str) - r2 - 1);
            } else {
                ZVAL_STR(&type, str);
                ZVAL_NULL(&subtype);
            }
            zend_update_property(Z_CE_P(val), val, ZEND_STRL("type"), &type);
            zend_update_property(Z_CE_P(val), val, ZEND_STRL("subtype"), &subtype);
        }
    } ZEND_HASH_FOREACH_END();
}

static inline void set_accept_language(zval *object, zval *server)
{
    zval val;
    zval *tmp;

    array_init(&val);
    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_ACCEPT_LANGUAGE"))) ) {
        php_request_parse_accept(&val, Z_STRVAL_P(tmp), Z_STRLEN_P(tmp));
        parse_accept_language(&val);
    }
    zend_update_property(Z_CE_P(object), object, ZEND_STRL("acceptLanguage"), &val);
}

static inline void set_auth(zval *object, zval *server)
{
    zval *tmp;
    zval digest = {0};

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("PHP_AUTH_PW"))) ) {
        zend_update_property(Z_CE_P(object), object, ZEND_STRL("authPw"), tmp);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("PHP_AUTH_TYPE"))) ) {
        zend_update_property(Z_CE_P(object), object, ZEND_STRL("authType"), tmp);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("PHP_AUTH_USER"))) ) {
        zend_update_property(Z_CE_P(object), object, ZEND_STRL("authUser"), tmp);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("PHP_AUTH_DIGEST"))) ) {
        zend_string *str = zval_get_string(tmp);
        php_request_parse_digest_auth(&digest, ZSTR_VAL(str), ZSTR_LEN(str));
        zend_update_property(Z_CE_P(object), object, ZEND_STRL("authDigest"), &digest);
    }

}

static inline void set_content(zval *object, zval *server)
{
    zval *tmp;
    php_stream *stream;
    zend_string *str;
    zval zv = {0};
    zval contentType = {0};

    // @todo read this when the property is read
    stream = php_stream_open_wrapper_ex("php://input", "rb", REPORT_ERRORS, NULL, NULL);
    if( stream ) {
        if ((str = php_stream_copy_to_mem(stream, PHP_STREAM_COPY_ALL, 0))) {
            ZVAL_STR(&zv, str);
            zend_update_property(Z_CE_P(object), object, ZEND_STRL("content"), &zv);
        }
        php_stream_close(stream);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_CONTENT_MD5"))) ) {
        zend_update_property(Z_CE_P(object), object, ZEND_STRL("contentMd5"), tmp);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_CONTENT_LENGTH"))) ) {
        zend_update_property(Z_CE_P(object), object, ZEND_STRL("contentLength"), tmp);
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_CONTENT_TYPE"))) && Z_TYPE_P(tmp) == IS_STRING ) {
        php_request_parse_content_type(&contentType, Z_STRVAL_P(tmp), Z_STRLEN_P(tmp));
        if( Z_TYPE(contentType) == IS_ARRAY ) {
            // contentType
            tmp = zend_hash_str_find(Z_ARRVAL(contentType), ZEND_STRL("value"));
            if( tmp ) {
                zend_update_property(Z_CE_P(object), object, ZEND_STRL("contentType"), tmp);
            }
            // charset
            tmp = zend_hash_str_find(Z_ARRVAL(contentType), ZEND_STRL("charset"));
            if( tmp ) {
                zend_update_property(Z_CE_P(object), object, ZEND_STRL("contentCharset"), tmp);
            }
        }
    }
}

PHP_METHOD(PhpRequest, __construct)
{
    zval *_this_zval;
    struct php_request_obj *intern;
    zval *server;
    zval *files;
    zval rv = {0};
    zval method = {0};
    zval headers = {0};
    zval uploads = {0};

    ZEND_PARSE_PARAMETERS_START(0, 0)
    ZEND_PARSE_PARAMETERS_END();

    _this_zval = getThis();
    intern = Z_REQUEST_P(_this_zval);

    // Copy superglobals
    copy_global_lit(_this_zval, "env", "_ENV");
    copy_global_lit(_this_zval, "server", "_SERVER");

    copy_global_lit(_this_zval, "cookie", "_COOKIE");
    copy_global_lit(_this_zval, "files", "_FILES");
    copy_global_lit(_this_zval, "get", "_GET");
    copy_global_lit(_this_zval, "post", "_POST");

    // Read back server property
    server = zend_read_property(Z_CE_P(_this_zval), _this_zval, ZEND_STRL("server"), 0, &rv);

    // Internal setters that require server
    if( server && Z_TYPE_P(server) == IS_ARRAY ) {
        // method
        ZVAL_STRING(&method, "");
        zend_bool xhr = php_request_detect_method(&method, server);
        zend_update_property(Z_CE_P(_this_zval), _this_zval, ZEND_STRL("method"), &method);
        zend_update_property_bool(Z_CE_P(_this_zval), _this_zval, ZEND_STRL("xhr"), xhr);

        // secure
        zend_bool secure = php_request_is_secure(server);
        zend_update_property_bool(Z_CE_P(_this_zval), _this_zval, ZEND_STRL("secure"), secure);

        // headers
        php_request_normalize_headers(&headers, server);
        zend_update_property(Z_CE_P(_this_zval), _this_zval, ZEND_STRL("headers"), &headers);

        // url
        set_url(_this_zval, server);

        // accepts
        set_accept_by_name(_this_zval, server, ZEND_STRL("HTTP_ACCEPT"), ZEND_STRL("acceptMedia"));
        set_accept_by_name(_this_zval, server, ZEND_STRL("HTTP_ACCEPT_CHARSET"), ZEND_STRL("acceptCharset"));
        set_accept_by_name(_this_zval, server, ZEND_STRL("HTTP_ACCEPT_ENCODING"), ZEND_STRL("acceptEncoding"));
        set_accept_language(_this_zval, server);

        // auth
        set_auth(_this_zval, server);
        set_content(_this_zval, server);
    }

    // Read back files property
    files = zend_read_property(Z_CE_P(_this_zval), _this_zval, ZEND_STRL("files"), 0, &rv);

    if( files && Z_TYPE_P(files) == IS_ARRAY ) {
        array_init(&uploads);
        php_request_normalize_files(&uploads, files);
        zend_update_property(Z_CE_P(_this_zval), _this_zval, ZEND_STRL("uploads"), &uploads);
    }

    // Lock the object
    intern->locked = 1;
}
/* }}} PhpRequest::__construct */

/* {{{ proto PhpRequest::parseAccept(string $header) */
PHP_METHOD(PhpRequest, parseAccept)
{
    zend_string *header;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(header)
    ZEND_PARSE_PARAMETERS_END();

    //array_init(return_value);
    php_request_parse_accept(return_value, ZSTR_VAL(header), ZSTR_LEN(header));

}
/* }}} PhpRequest::parseAccept */

/* {{{ proto PhpRequest::parseContentType(string $header) */
PHP_METHOD(PhpRequest, parseContentType)
{
    zend_string *header;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(header)
    ZEND_PARSE_PARAMETERS_END();

    php_request_parse_content_type(return_value, ZSTR_VAL(header), ZSTR_LEN(header));
}
/* }}} PhpRequest::parseContentType */

/* {{{ proto PhpRequest::parseDigestAuth(string $header) */
PHP_METHOD(PhpRequest, parseDigestAuth)
{
    zend_string *header;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(header)
    ZEND_PARSE_PARAMETERS_END();

    php_request_parse_digest_auth(return_value, ZSTR_VAL(header), ZSTR_LEN(header));
}
/* }}} PhpRequest::parseDigestAuth */

/* {{{ PhpRequest methods */
static zend_function_entry PhpRequest_methods[] = {
    PHP_ME(PhpRequest, __construct, PhpRequest_construct_args, ZEND_ACC_PUBLIC)
    PHP_ME(PhpRequest, parseAccept, PhpRequest_parseAccept_args, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(PhpRequest, parseContentType, PhpRequest_parseContentType_args, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(PhpRequest, parseDigestAuth, PhpRequest_parseDigestAuth_args, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_FE_END
};
/* }}} PhpRequest methods */

/* {{{ PHP_MINIT_FUNCTION */
static PHP_MINIT_FUNCTION(request)
{
    zend_class_entry ce;

    memcpy(&PhpRequest_obj_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    PhpRequest_obj_handlers.offset = XtOffsetOf(struct php_request_obj, std);
    PhpRequest_obj_handlers.has_property = php_request_object_has_property;
    PhpRequest_obj_handlers.read_property = php_request_object_read_property;
    PhpRequest_obj_handlers.write_property = php_request_object_write_property;
    PhpRequest_obj_handlers.unset_property = php_request_object_unset_property;
    PhpRequest_obj_handlers.get_property_ptr_ptr = NULL;

    INIT_CLASS_ENTRY(ce, "PhpRequest", PhpRequest_methods);
    PhpRequest_ce_ptr = zend_register_internal_class(&ce);
    PhpRequest_ce_ptr->create_object = php_request_obj_create;

    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("acceptCharset"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("acceptEncoding"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("acceptLanguage"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("acceptMedia"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("authDigest"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("authPw"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("authType"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("authUser"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("content"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("contentCharset"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("contentLength"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("contentMd5"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("contentType"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("cookie"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("env"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("files"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("get"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("headers"), ZEND_ACC_PUBLIC);
    zend_declare_property_string(PhpRequest_ce_ptr, ZEND_STRL("method"), "", ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("post"), ZEND_ACC_PUBLIC);
    zend_declare_property_bool(PhpRequest_ce_ptr, ZEND_STRL("secure"), 0, ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("server"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("uploads"), ZEND_ACC_PUBLIC);
    zend_declare_property_null(PhpRequest_ce_ptr, ZEND_STRL("url"), ZEND_ACC_PUBLIC);
    zend_declare_property_bool(PhpRequest_ce_ptr, ZEND_STRL("xhr"), 0, ZEND_ACC_PUBLIC);

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

zend_module_entry request_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_REQUEST_NAME,                   /* Name */
    NULL,                               /* Functions */
    PHP_MINIT(request),                 /* MINIT */
    NULL,                               /* MSHUTDOWN */
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