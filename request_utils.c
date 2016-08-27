
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
zend_bool php_request_detect_method(zval *return_value, zval *server)
{
    zval rv;
    zend_string* tmp;
    zval* val;
    zend_bool xhr = 0;
    zend_string *method;

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
        tmp = zend_string_dup(method, 0);
        php_strtoupper(ZSTR_VAL(tmp), ZSTR_LEN(tmp));
        ZVAL_STR(return_value, tmp);
    }

    return xhr;
}
/* }}} */

/* {{{ php_request_detect_url */
static inline const unsigned char * extract_port_from_host(const unsigned char * host, size_t len)
{
    const unsigned char * right = host + len - 1;
    const unsigned char * left = len > 6 ? right - 6 : host;
    const unsigned char * pos = right;
    for( ; pos > left; pos-- ) {
        if( !isdigit(*pos) ) {
            if( *pos == ':' ) {
                return pos + 1;
            }
            break;
        }
    }
    return NULL;
}

static inline zend_string *extract_host_from_server(zval *server)
{
    zval *tmp;
    zend_string *host = NULL;

    // Get host
    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("HTTP_HOST"))) &&
            Z_TYPE_P(tmp) == IS_STRING ) {
        host = Z_STR_P(tmp);
    } else if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("SERVER_NAME"))) &&
            Z_TYPE_P(tmp) == IS_STRING ) {
        host = Z_STR_P(tmp);
    }

    return host;
}

static inline zend_long extract_port_from_server(zval *server, zend_string * host)
{
    zval *tmp;

    // Get port
    if( NULL != extract_port_from_host(ZSTR_VAL(host), ZSTR_LEN(host)) ) {
        // no need to extract
    } else if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("SERVER_PORT"))) ) {
        return zval_get_long(tmp);
    }

    return 0;
}

static inline zend_string *extract_uri_from_server(zval *server)
{
    zval *tmp;

    if( (tmp = zend_hash_str_find(Z_ARRVAL_P(server), ZEND_STRL("REQUEST_URI"))) &&
            Z_TYPE_P(tmp) == IS_STRING ) {
        return Z_STR_P(tmp);
    }

    return NULL;
}

zend_string *php_request_detect_url(zval *server)
{
    zval *tmp;
    zend_bool is_secure = php_request_is_secure(server);
    zend_string * host = NULL;
    zend_long port = 0;
    zend_string * uri = NULL;
    smart_str buf = {0};
    php_url * url;
    zval arr = {0};

    // Get host
    if( !(host = extract_host_from_server(server)) ) {
        return NULL;
    }

    port = extract_port_from_server(server, host);

    // Get uri
    uri = extract_uri_from_server(server);

    // Form URL
    smart_str_alloc(&buf, 1024, 0);
    if( is_secure ) {
        smart_str_appendl_ex(&buf, ZEND_STRL("https://"), 0);
    } else {
        smart_str_appendl_ex(&buf, ZEND_STRL("http://"), 0);
    }
    smart_str_append_ex(&buf, host, 0);
    if( port > 0 ) {
        smart_str_appendc_ex(&buf, ':', 0);
        smart_str_append_long(&buf, port);
    }
    if( uri ) {
        smart_str_append_ex(&buf, uri, 0);
    }
    smart_str_0(&buf);

    return buf.s;
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

/* {{{ php_request_normalize_files */
static void upload_from_spec(zval *return_value, zval *file);

static inline void copy_key(zval *return_value, zval *nested, const char *key1, size_t key1_len, zend_ulong index2, zend_string *key2)
{
    zval *tmp = zend_hash_str_find(Z_ARRVAL_P(nested), key1, key1_len);
    if( !tmp ) {
        goto err;
    }

    if( Z_TYPE_P(tmp) == IS_ARRAY ) {
        if( key2 ) {
            tmp = zend_hash_find(Z_ARRVAL_P(tmp), key2);
        } else {
            tmp = zend_hash_index_find(Z_ARRVAL_P(tmp), index2);
        }
    }

    if( tmp ) {
        add_assoc_zval_ex(return_value, key1, key1_len, tmp);
    } else {
        err:
        add_assoc_null_ex(return_value, key1, key1_len);
    }
}

static inline void upload_from_nested(zval *return_value, zval *nested, zval *tmp_name)
{
    zend_string *key;
    zend_ulong index;
    zval tmp = {0};
    zval tmp2 = {0};

    array_init(return_value);

    ZEND_HASH_FOREACH_KEY(Z_ARRVAL_P(tmp_name), index, key) {
        array_init(&tmp);
        copy_key(&tmp, nested, ZEND_STRL("error"), index, key);
        copy_key(&tmp, nested, ZEND_STRL("name"), index, key);
        copy_key(&tmp, nested, ZEND_STRL("size"), index, key);
        copy_key(&tmp, nested, ZEND_STRL("tmp_name"), index, key);
        copy_key(&tmp, nested, ZEND_STRL("type"), index, key);

        upload_from_spec(&tmp2, &tmp);
        if( key ) {
            add_assoc_zval_ex(return_value, ZSTR_VAL(key), ZSTR_LEN(key), &tmp2);
        } else {
            add_index_zval(return_value, index, &tmp2);
        }
    } ZEND_HASH_FOREACH_END();
}

static void upload_from_spec(zval *return_value, zval *file)
{
    zval *tmp = zend_hash_str_find(Z_ARRVAL_P(file), ZEND_STRL("tmp_name"));
    if( tmp && Z_TYPE_P(tmp) == IS_ARRAY ) {
        upload_from_nested(return_value, file, tmp);
    } else {
        ZVAL_ZVAL(return_value, file, 0, 0);
        convert_to_object(return_value);
    }
}

void php_request_normalize_files(zval *return_value, zval *files)
{
    zend_string *key;
    zend_ulong index;
    zval *val;
    zval tmp = {0};

    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(files), index, key, val) {
        if( Z_TYPE_P(val) == IS_ARRAY ) {
            upload_from_spec(&tmp, val);
            if( key ) {
                add_assoc_zval_ex(return_value, ZSTR_VAL(key), ZSTR_LEN(key), &tmp);
            } else {
                add_index_zval(return_value, index, &tmp);
            }
        }
    } ZEND_HASH_FOREACH_END();
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
