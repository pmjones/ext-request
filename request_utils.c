
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "main/php.h"
#include "ext/standard/php_string.h"
#include "ext/standard/url.h"
#include "Zend/zend_API.h"
#include "Zend/zend_portability.h"
#include "Zend/zend_smart_str.h"

#include "php_request.h"
#include "request_utils.h"

/* {{ php_request_detect_method */
zend_bool php_request_detect_method(zval *return_value, zval *server, zend_string *method)
{
    zval rv;
    zend_string* tmp;
    zval* val;
    zend_bool xhr = 0;

    // force the method?
    if( method && ZSTR_LEN(method) > 0 ) {
        goto found;
    }

    // check server
    if( !server || Z_TYPE_P(server) != IS_ARRAY ) {
        return xhr;
    }

    // determine method from request
    val = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("REQUEST_METHOD"));
    if( !val || Z_TYPE_P(val) != IS_STRING ) {
        return xhr;
    }
    method = Z_STR_P(val);

    // XmlHttpRequest method override?
    if( zend_string_equals_literal_ci(method, "POST") ) {
        val = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_X_HTTP_METHOD_OVERRIDE"));
        if( val && Z_TYPE_P(val) == IS_STRING ) {
            method = Z_STR_P(val);
            xhr = 1;
        }
    }

    if( method ) {
        found:
        tmp = zend_string_dup(method, 0);
        php_strtoupper(ZSTR_VAL(tmp), ZSTR_LEN(tmp));
        ZVAL_STR(return_value, tmp);
        zend_string_release(tmp);
    }

    return xhr;
}
/* }}} */

/* {{{ php_request_is_secure */
zend_bool php_request_is_secure(zval *server)
{
    zval *tmp;

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTPS"))) &&
        Z_TYPE_P(tmp) == IS_STRING &&
        zend_string_equals_literal_ci(Z_STR_P(tmp), "on") ) {
        return 1;
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("SERVER_PORT"))) &&
            443 == zval_get_long(tmp) ) {
        return 1;
    }

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_X_FORWARDED_PROTO"))) &&
               Z_TYPE_P(tmp) == IS_STRING &&
               zend_string_equals_literal_ci(Z_STR_P(tmp), "https") ) {
        return 1;
    }

    return 0;
}
/* }}} */

/* {{{ php_request_normalize_headers */
static const char http_str[] = "HTTP_";
static const size_t http_len = sizeof(http_str) - 1;

void php_request_normalize_header_name(char *key, size_t key_length)
{
    register char *r = key;
    register char *r_end = r_end = r + key_length - 1;

    *r = toupper((unsigned char) *r);
    r++;
    for( ; r <= r_end; r++ ) {
        if( (unsigned char)*(r - 1) == '-' ) {
            *r = toupper((unsigned char) *r);
        } else if( *r == '_' ) {
            *r = '-';
        } else {
            *r = tolower((unsigned char) *r);
        }
    }
}

void php_request_normalize_headers(zval *return_value, zval *server)
{
    zend_string *key;
    zend_ulong index;
    zval *val;
    zend_string *tmp;

    array_init(return_value);

    // Main headers
    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(server), index, key, val) {
        if( key && ZSTR_LEN(key) > 5 && strncmp(ZSTR_VAL(key), http_str, http_len) == 0 ) {
            tmp = zend_string_init(ZSTR_VAL(key) + http_len, ZSTR_LEN(key) - http_len, 0);
            php_request_normalize_header_name(ZSTR_VAL(tmp), ZSTR_LEN(tmp));
            add_assoc_zval_ex(return_value, ZSTR_VAL(tmp), ZSTR_LEN(tmp), val);
            zend_string_release(tmp);
        }
    } ZEND_HASH_FOREACH_END();

    // RFC 3875 headers not prefixed with HTTP_*
    if( val = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("CONTENT_LENGTH")) ) {
        add_assoc_zval_ex(return_value, ZEND_STRL("Content-Length"), val);
    }
    if( val = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("CONTENT_TYPE")) ) {
        add_assoc_zval_ex(return_value, ZEND_STRL("Content-Type"), val);
    }
}
/* }}} */

/* {{{ php_request_parse_accepts */
struct accepts_ctx {
    int st;
    zval value;
    zval quality;
    zval params;
};

static int php_request_accept_compare(const void *a, const void *b)
{
    Bucket *f = (Bucket *) a;
    Bucket *s = (Bucket *) b;
    zval *first = &f->val;
    zval *second = &s->val;

    if( Z_TYPE_P(first) != IS_OBJECT || Z_TYPE_P(second) != IS_OBJECT ) {
        return 0;
    }

    first = zend_read_property(Z_CE_P(first), first, ZEND_STRL("quality"), 0, NULL);
    second = zend_read_property(Z_CE_P(second), second, ZEND_STRL("quality"), 0, NULL);

    if( !first || !second || Z_TYPE_P(first) != IS_STRING || Z_TYPE_P(second) != IS_STRING ) {
        return 0;
    }

    return -1 * strnatcmp_ex(Z_STRVAL_P(first), Z_STRLEN_P(first), Z_STRVAL_P(second), Z_STRLEN_P(second), 0);
}

static void php_request_parse_accepts4(struct accepts_ctx *ctx, const unsigned char * str, size_t len)
{
    const unsigned char * end = str + len;
    const unsigned char * pos = str;

    while( *pos != '=' && pos++ < end );

    if( pos >= end ) {
        // no equals
    } else {
        assert(pos >= str);
        assert(end > pos);

        const char * key = str;
        size_t key_len = pos - str;
        const char * value = pos + 1;
        size_t value_len = end - pos - 1;

        if( key_len == 1 && *key == 'q' ) {
            ZVAL_STRINGL(&ctx->quality, value, value_len);
        } else {
            add_assoc_stringl_ex(&ctx->params, key, key_len, value, value_len);
        }
    }
}

static void php_request_parse_accepts3(struct accepts_ctx *ctx, const unsigned char * str, size_t len)
{
    if( ctx->st == 0 ) {
        zend_string * v2 = zend_string_init(str, len, 0);
        zend_string * v3 = php_trim(v2, ZEND_STRL(" \t\n\r\v"), 3);
        ZVAL_STR(&ctx->value, v3);
//        zend_string_release(v3);
        zend_string_release(v2);
//        ZVAL_STRINGL(&ctx->value, str, len);
        ctx->st = 1;
    } else {
        php_request_parse_accepts4(ctx, str, len);
    }
}

static void php_request_parse_accepts2(zval * return_value, struct accepts_ctx *ctx, const unsigned char * str, size_t len)
{
    php_request_parse_accepts3(ctx, str, len);

    // Make return value
    zval item;
    array_init(&item);
    add_assoc_zval_ex(&item, ZEND_STRL("value"), &ctx->value);
    add_assoc_zval_ex(&item, ZEND_STRL("quality"), &ctx->quality);
    add_assoc_zval_ex(&item, ZEND_STRL("params"), &ctx->params);
    convert_to_object(&item);
    add_next_index_zval(return_value, &item);

    // Reset
    memset(ctx, 0, sizeof(struct accepts_ctx));
    ZVAL_UNDEF(&ctx->value);
    array_init(&ctx->params);
    ZVAL_STRING(&ctx->quality, "1.0");
    // @todo this might leak
}

void php_request_parse_accepts(zval * return_value, const unsigned char * str, size_t len)
{
    const unsigned char * end = str + len;
    const unsigned char * pos = str;
    const unsigned char * prev = str;
    struct accepts_ctx ctx = {0};

    ZVAL_UNDEF(&ctx.value);
    array_init(&ctx.params);
    ZVAL_STRING(&ctx.quality, "1.0");

    for( ; pos < end; pos++ ) {
        if( *pos == ',' ) {
            assert(pos >= prev);
            php_request_parse_accepts2(return_value, &ctx, prev, pos - prev);
            prev = pos + 1;
        } else if( *pos == ';' ) {
            assert(pos >= prev);
            php_request_parse_accepts3(&ctx, prev, pos - prev);
            prev = pos + 1;
        }
    }

    php_request_parse_accepts2(return_value, &ctx, prev, pos - prev);
    zend_hash_sort(Z_ARRVAL_P(return_value), php_request_accept_compare, 1);
}
/* }}} */
