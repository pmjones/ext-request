
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>

#include "main/php.h"
#include "ext/standard/php_string.h"
#include "Zend/zend_API.h"
#include "Zend/zend_smart_str.h"

#include "php_request.h"
#include "request_parsers.h"

/* Adapted from http://re2c.org/examples/example_07.html */

typedef unsigned char YYCTYPE;

/*!re2c
    re2c:define:YYCURSOR = in->cur;
    re2c:define:YYMARKER = in->mar;
    re2c:define:YYLIMIT = in->lim;
    re2c:yyfill:enable = 1;
    re2c:define:YYFILL = "{ token1(&tok, TOKEN_INVALID, \"\", 0); return tok; }";
    re2c:define:YYFILL:naked = 1;

    end = "\x00";
    id = [a-zA-Z0-9_\.-]+;
    mime = ("*" | [a-zA-Z0-9-]+) "/" ("*" | [a-zA-Z0-9-]+);
*/
/*!max:re2c*/

enum scanner_token_type {
    TOKEN_END = 0,
    TOKEN_INVALID,
    TOKEN_WHITESPACE,
    TOKEN_STRING,
    TOKEN_EQUALS,
    TOKEN_COMMA,
    TOKEN_ID,
    TOKEN_SEMICOLON,
    TOKEN_SLASH,
    TOKEN_MIME,
    TOKEN_STAR
};

struct scanner_input {
    const YYCTYPE *buf;
    const YYCTYPE *tok;
    const YYCTYPE *cur;
    const YYCTYPE *mar;
    const YYCTYPE *lim;
};

struct scanner_token {
    enum scanner_token_type type;
    const YYCTYPE *yytext;
    size_t yyleng;
};

static zend_string *strip_slashes(const unsigned char *str, size_t len)
{
    register const YYCTYPE *pos = str;
    register const YYCTYPE *end = str + len;
    smart_str buf = {0};
    smart_str_alloc(&buf, len, 0);
    for( ; pos != end; pos++ ) {
        if( *pos != '\\' ) {
            smart_str_appendc_ex(&buf, *pos, 0);
        }
    }
    return buf.s;
}

static inline void token1(struct scanner_token *tok, enum scanner_token_type type,  const YYCTYPE *yytext, size_t yyleng)
{
    tok->type = type;
    tok->yytext = yytext;
    tok->yyleng = yyleng;
}

static struct scanner_token lex_quoted_str(struct scanner_input *in, YYCTYPE q)
{
    struct scanner_token tok = {0};
    const YYCTYPE *start = in->cur;

    YYCTYPE u = q;
    for (;;) {
        in->tok = in->cur;
        /*!re2c
            *                    { token1(&tok, TOKEN_INVALID, "", 0); return tok; }
            end                  { token1(&tok, TOKEN_INVALID, "", 0); return tok; }
            [^\x00]              { u = *in->tok; if (u == q) break; continue; }
            "\\" [^\x00]         { u = *(in->cur - 1); }
        */
    }
    token1(&tok, TOKEN_STRING, start, in->tok - start);
    return tok;
}

/* {{{ php_request_lex_generic */
static struct scanner_token lex(struct scanner_input *in)
{
    struct scanner_token tok = {0};
    for( ;; ) {
        in->tok = in->cur;
        /*!re2c
            *   { token1(&tok, TOKEN_INVALID, in->tok, 1); break; }
            end { token1(&tok, TOKEN_END, "", 0); break; }

            // whitespaces
            [ \t\v\n\r] { continue; tok.type = TOKEN_WHITESPACE; break; }

            // character and string literals
            ['"] { tok = lex_quoted_str(in, *(in->cur - 1)); break; }
            "''" { token1(&tok, TOKEN_STRING, "", 0); break; }

            // operators
            "=" { token1(&tok, TOKEN_EQUALS, in->tok, 1); break; }
            "/" { token1(&tok, TOKEN_SLASH, in->tok, 1); break; }
            ";" { token1(&tok, TOKEN_SEMICOLON, in->tok, 1); break; }
            "," { token1(&tok, TOKEN_COMMA, in->tok, 1); break; }
            "*" { token1(&tok, TOKEN_STAR, in->tok, 1); break; }

            // identifiers
            mime { token1(&tok, TOKEN_MIME, in->tok, in->cur - in->tok); break; }
            id { token1(&tok, TOKEN_ID, in->tok, in->cur - in->tok); break; }
        */
    }
    //fprintf(stderr, "TOKEN[%d] %.*s\n", tok.type, tok.yyleng, tok.yytext);
    return tok;
}
/* }}} php_request_lex_generic */

/* {{{ php_request_parse_accept */
/* @see https://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html */
static int php_request_accept_compare(const void *a, const void *b)
{
    Bucket *f = (Bucket *) a;
    Bucket *s = (Bucket *) b;
    zval *first = &f->val;
    zval *second = &s->val;

    if( Z_TYPE_P(first) != IS_ARRAY || Z_TYPE_P(second) != IS_ARRAY ) {
        return 0;
    }

    first = zend_hash_str_find(Z_ARRVAL_P(first), ZEND_STRL("quality"));
    second = zend_hash_str_find(Z_ARRVAL_P(second), ZEND_STRL("quality"));

    if( !first || !second || Z_TYPE_P(first) != IS_STRING || Z_TYPE_P(second) != IS_STRING ) {
        return 0;
    }

    return -1 * strnatcmp_ex(Z_STRVAL_P(first), Z_STRLEN_P(first), Z_STRVAL_P(second), Z_STRLEN_P(second), 0);
}

static int parse_accept_params(struct scanner_input *in, zval *params)
{
    struct scanner_token tok;
    struct scanner_token left;
    struct scanner_token right;
    zend_string *value;

    array_init(params);

    for(;;) {
        // Check for semicolon or comma
        tok = lex(in);
        if( tok.type == TOKEN_SEMICOLON ) {
            // there's another param
        } else if( tok.type == TOKEN_COMMA ) {
            // end of params
            break;
        } else {
            return 0; // err
        }

        // ID
        tok = lex(in);
        if( tok.type != TOKEN_ID ) {
            return 0; // err
        }
        left = tok;

        // Equals
        tok = lex(in);
        if( tok.type != TOKEN_EQUALS ) {
            return 0; // err
        }

        // ID | string
        tok = lex(in);
        if( tok.type != TOKEN_ID && tok.type != TOKEN_STRING ) {
            return 0; // err
        }
        right = tok;

        // Save KV pair
        if( right.type == TOKEN_STRING ) {
            value = strip_slashes(right.yytext, right.yyleng);
        } else {
            value = zend_string_init(right.yytext, right.yyleng, 0);
        }
        add_assoc_str_ex(params, left.yytext, left.yyleng, value);
    }

    return 1;
}

void php_request_parse_accept(zval *return_value, const YYCTYPE *str, size_t len)
{
    // Pad the buffer
    YYCTYPE *str2 = emalloc(len + YYMAXFILL + 1);
    memcpy(str2, str, len);
    memset(str2 + len, 0, YYMAXFILL + 1);

    struct scanner_input in = {
        str2,
        str2,
        str2,
        0,
        str2 + len + YYMAXFILL
    };
    struct scanner_token tok = {0};
    zval *qual;
    int con = 1;

    array_init(return_value);

    for(;con;) {
        zval item = {0};
        zval params = {0};
        int ret;

        // MIME type
        tok = lex(&in);
        if( tok.type != TOKEN_MIME && tok.type != TOKEN_ID && tok.type != TOKEN_STAR ) {
            break;
        }

        // Params
        con = parse_accept_params(&in, &params);

        // Save
        array_init(&item);
        add_assoc_stringl_ex(&item, ZEND_STRL("value"), (char *) tok.yytext, tok.yyleng);

        // Get quality
        qual = zend_hash_str_find(Z_ARRVAL(params), ZEND_STRL("q"));
        if( qual && Z_TYPE_P(qual) == IS_STRING ) {
            add_assoc_stringl_ex(&item, ZEND_STRL("quality"), Z_STRVAL_P(qual), Z_STRLEN_P(qual));
            zend_hash_str_del(Z_ARRVAL(params), ZEND_STRL("q"));
        } else {
            add_assoc_string_ex(&item, ZEND_STRL("quality"), "1.0");
        }

        add_assoc_zval_ex(&item, ZEND_STRL("params"), &params);
        add_next_index_zval(return_value, &item);
    };

    // Sort
    zend_hash_sort(Z_ARRVAL_P(return_value), php_request_accept_compare, 1);

    efree(str2);
}
/* }}} php_request_parse_accept */

/* {{{ php_request_parse_content_type */
/* @see https://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html#sec3.7 */
/* @see https://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html */
void php_request_parse_content_type(zval *return_value, const YYCTYPE *str, size_t len)
{
    // Pad the buffer
    YYCTYPE *str2 = emalloc(len + YYMAXFILL + 1);
    memcpy(str2, str, len);
    memset(str2 + len, 0, YYMAXFILL + 1);

    struct scanner_input in = {
        str2,
        str2,
        str2,
        0,
        str2 + len + YYMAXFILL
    };
    struct scanner_token tok = {0};
    struct scanner_token left;
    struct scanner_token right;
    zend_string *value;
    zval params;

    // Read mime
    tok = lex(&in);
    if( tok.type != TOKEN_MIME ) {
        goto err;
    }

    array_init(return_value);
    add_assoc_stringl_ex(return_value, ZEND_STRL("value"), (char *) tok.yytext, tok.yyleng);

    // Read params
    array_init(&params);
    for(;;) {
        // Read comma
        tok = lex(&in);
        if( tok.type != TOKEN_SEMICOLON ) {
            break;
        }

        // Read left
        tok = lex(&in);
        if( tok.type != TOKEN_ID ) {
            break;
        }
        left = tok;

        // Read equals
        tok = lex(&in);
        if( tok.type != TOKEN_EQUALS ) {
            break;
        }

        // Read right
        tok = lex(&in);
        if( tok.type != TOKEN_ID && tok.type != TOKEN_STRING ) {
            break;
        }
        right = tok;

        // Save KV pair
        if( right.type == TOKEN_STRING ) {
            value = strip_slashes(right.yytext, right.yyleng);
        } else {
            value = zend_string_init(right.yytext, right.yyleng, 0);
        }

        if( left.yyleng == sizeof("charset") - 1 && 0 == strncmp(left.yytext, "charset", sizeof("charset") - 1) ) {
            add_assoc_str_ex(return_value, left.yytext, left.yyleng, value);
        } else {
            add_assoc_str_ex(&params, left.yytext, left.yyleng, value);
        }
    }

    add_assoc_zval_ex(return_value, ZEND_STRL("params"), &params);

err:
    efree(str2);
}
/* }}} php_request_parse_content_type */

/* {{{ php_request_parse_digest_auth */
/* @see: https://secure.php.net/manual/en/features.http-auth.php */
/* @see: https://www.w3.org/Protocols/rfc2616/rfc2616-sec2.html */
/* @see: https://en.wikipedia.org/wiki/Digest_access_authentication */
void php_request_parse_digest_auth(zval *return_value, const YYCTYPE *str, size_t len)
{
    // Pad the buffer
    YYCTYPE *str2 = emalloc(len + YYMAXFILL + 1);
    memcpy(str2, str, len);
    memset(str2 + len, 0, YYMAXFILL + 1);

    struct scanner_input in = {
        str2,
        str2,
        str2,
        0,
        str2 + len + YYMAXFILL
    };
    struct scanner_token tok;
    struct scanner_token left;
    struct scanner_token right;
    zval need;
    zend_string *value;

    // Build need array
    array_init(&need);
    add_assoc_bool_ex(&need, ZEND_STRL("nonce"), 1);
    add_assoc_bool_ex(&need, ZEND_STRL("nc"), 1);
    add_assoc_bool_ex(&need, ZEND_STRL("cnonce"), 1);
    add_assoc_bool_ex(&need, ZEND_STRL("qop"), 1);
    add_assoc_bool_ex(&need, ZEND_STRL("username"), 1);
    add_assoc_bool_ex(&need, ZEND_STRL("uri"), 1);
    add_assoc_bool_ex(&need, ZEND_STRL("response"), 1);

    // Parse digest auth
    array_init(return_value);
    for( ;; ) {
        // Read ID
        tok = lex(&in);
        if( tok.type != TOKEN_ID ) {
            break;
        }
        left = tok;

        // Read equals
        tok = lex(&in);
        if( tok.type != TOKEN_EQUALS ) {
            break;
        }

        // Read ID | string
        tok = lex(&in);
        if( tok.type != TOKEN_ID && tok.type != TOKEN_STRING ) {
            break;
        }
        right = tok;

        // Save KV pair
        if( right.type == TOKEN_STRING ) {
            value = strip_slashes(right.yytext, right.yyleng);
        } else {
            value = zend_string_init(right.yytext, right.yyleng, 0);
        }
        add_assoc_str_ex(return_value, left.yytext, left.yyleng, value);
        zend_hash_str_del(Z_ARRVAL_P(&need), left.yytext, left.yyleng);

        // Read comma | end
        tok = lex(&in);
        if( tok.type != TOKEN_COMMA ) {
            break;
        }
    };

    if( zend_array_count(Z_ARRVAL_P(&need)) > 0 ) {
        // blow result away if invalid
        ZVAL_NULL(return_value);
    }

    zval_dtor(&need);
    efree(str2);
}
/* }}} php_request_parse_digest_auth */
