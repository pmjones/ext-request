
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "main/php.h"
#include "ext/standard/php_string.h"
#include "Zend/zend_API.h"
#include "Zend/zend_portability.h"

#include "php_request.h"
#include "request_utils.h"

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

    return 1 - strnatcmp_ex(Z_STRVAL_P(first), Z_STRLEN_P(first), Z_STRVAL_P(second), Z_STRLEN_P(second), 0);
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
