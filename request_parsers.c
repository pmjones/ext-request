/* Generated by re2c 0.16 */
#line 1 "request_parsers.re"

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

#define lex php_request_lex_generic
#define strip_slashes php_request_strip_slashes
#define token1 php_request_parser_token_init

#line 36 "request_parsers.re"

#define YYMAXFILL 3

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

static inline void token1(struct scanner_token *tok, enum scanner_token_type type, const YYCTYPE *yytext, size_t yyleng)
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
        
#line 90 "request_parsers.c"
{
	YYCTYPE yych;
	if ((in->lim - in->cur) < 2) { token1(&tok, TOKEN_INVALID, "", 0); return tok; }
	yych = *in->cur;
	switch (yych) {
	case 0x00:	goto yy2;
	case '\\':	goto yy6;
	default:	goto yy4;
	}
yy2:
	++in->cur;
#line 98 "request_parsers.re"
	{ token1(&tok, TOKEN_INVALID, "", 0); return tok; }
#line 104 "request_parsers.c"
yy4:
	++in->cur;
yy5:
#line 99 "request_parsers.re"
	{ u = *in->tok; if (u == q) break; continue; }
#line 110 "request_parsers.c"
yy6:
	yych = *++in->cur;
	if (yych <= 0x00) goto yy5;
	++in->cur;
#line 100 "request_parsers.re"
	{ u = *(in->cur - 1); }
#line 117 "request_parsers.c"
}
#line 101 "request_parsers.re"

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
        
#line 133 "request_parsers.c"
{
	YYCTYPE yych;
	unsigned int yyaccept = 0;
	if ((in->lim - in->cur) < 3) { token1(&tok, TOKEN_INVALID, "", 0); return tok; }
	yych = *in->cur;
	switch (yych) {
	case 0x00:	goto yy11;
	case '\t':
	case '\n':
	case '\v':
	case '\r':
	case ' ':	goto yy15;
	case '"':	goto yy17;
	case '\'':	goto yy19;
	case '*':	goto yy20;
	case ',':	goto yy22;
	case '-':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy24;
	case '.':
	case '_':	goto yy27;
	case '/':	goto yy29;
	case ';':	goto yy31;
	case '=':	goto yy33;
	default:	goto yy13;
	}
yy11:
	++in->cur;
#line 115 "request_parsers.re"
	{ token1(&tok, TOKEN_END, "", 0); break; }
#line 224 "request_parsers.c"
yy13:
	++in->cur;
#line 114 "request_parsers.re"
	{ token1(&tok, TOKEN_INVALID, in->tok, 1); break; }
#line 229 "request_parsers.c"
yy15:
	++in->cur;
#line 118 "request_parsers.re"
	{ continue; tok.type = TOKEN_WHITESPACE; break; }
#line 234 "request_parsers.c"
yy17:
	++in->cur;
yy18:
#line 121 "request_parsers.re"
	{ tok = lex_quoted_str(in, *(in->cur - 1)); break; }
#line 240 "request_parsers.c"
yy19:
	yych = *++in->cur;
	switch (yych) {
	case '\'':	goto yy35;
	default:	goto yy18;
	}
yy20:
	yyaccept = 0;
	yych = *(in->mar = ++in->cur);
	switch (yych) {
	case '/':	goto yy37;
	default:	goto yy21;
	}
yy21:
#line 129 "request_parsers.re"
	{ token1(&tok, TOKEN_STAR, in->tok, 1); break; }
#line 257 "request_parsers.c"
yy22:
	++in->cur;
#line 128 "request_parsers.re"
	{ token1(&tok, TOKEN_COMMA, in->tok, 1); break; }
#line 262 "request_parsers.c"
yy24:
	yyaccept = 1;
	in->mar = ++in->cur;
	if ((in->lim - in->cur) < 2) { token1(&tok, TOKEN_INVALID, "", 0); return tok; }
	yych = *in->cur;
	switch (yych) {
	case '-':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy24;
	case '.':
	case '_':	goto yy27;
	case '/':	goto yy37;
	default:	goto yy26;
	}
yy26:
#line 133 "request_parsers.re"
	{ token1(&tok, TOKEN_ID, in->tok, in->cur - in->tok); break; }
#line 340 "request_parsers.c"
yy27:
	++in->cur;
	if (in->lim <= in->cur) { token1(&tok, TOKEN_INVALID, "", 0); return tok; }
	yych = *in->cur;
	switch (yych) {
	case '-':
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy27;
	default:	goto yy26;
	}
yy29:
	++in->cur;
#line 126 "request_parsers.re"
	{ token1(&tok, TOKEN_SLASH, in->tok, 1); break; }
#line 417 "request_parsers.c"
yy31:
	++in->cur;
#line 127 "request_parsers.re"
	{ token1(&tok, TOKEN_SEMICOLON, in->tok, 1); break; }
#line 422 "request_parsers.c"
yy33:
	++in->cur;
#line 125 "request_parsers.re"
	{ token1(&tok, TOKEN_EQUALS, in->tok, 1); break; }
#line 427 "request_parsers.c"
yy35:
	++in->cur;
#line 122 "request_parsers.re"
	{ token1(&tok, TOKEN_STRING, "", 0); break; }
#line 432 "request_parsers.c"
yy37:
	yych = *++in->cur;
	switch (yych) {
	case '*':	goto yy39;
	case '-':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy41;
	default:	goto yy38;
	}
yy38:
	in->cur = in->mar;
	if (yyaccept == 0) {
		goto yy21;
	} else {
		goto yy26;
	}
yy39:
	++in->cur;
yy40:
#line 132 "request_parsers.re"
	{ token1(&tok, TOKEN_MIME, in->tok, in->cur - in->tok); break; }
#line 514 "request_parsers.c"
yy41:
	++in->cur;
	if (in->lim <= in->cur) { token1(&tok, TOKEN_INVALID, "", 0); return tok; }
	yych = *in->cur;
	switch (yych) {
	case '-':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy41;
	default:	goto yy40;
	}
}
#line 134 "request_parsers.re"

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
