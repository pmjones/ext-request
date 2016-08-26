
#include <stdio.h>
#include <string.h>

#define YYCTYPE char

struct lex_ctx {
    unsigned char * buf;
    unsigned char * tok;
    unsigned char * cur;
    unsigned char * mar;
    unsigned char * lim;
};

static int lex_str(struct lex_ctx *in, unsigned char q)
{
    printf("%c", q);
    unsigned char u = q;
    for (;;printf("%c", u)) {
        in->tok = in->cur;
        /*!re2c
            re2c:define:YYCURSOR = in->cur;
            re2c:define:YYMARKER = in->mar;
            re2c:define:YYLIMIT = in->lim;
            re2c:yyfill:enable = 0;

            *                    { return 0; }
            [^\n\\]              { u = *in->tok; if (u == q) break; continue; }
            "\\a"                { u = '\a'; continue; }
            "\\b"                { u = '\b'; continue; }
            "\\f"                { u = '\f'; continue; }
            "\\n"                { u = '\n'; continue; }
            "\\r"                { u = '\r'; continue; }
            "\\t"                { u = '\t'; continue; }
            "\\v"                { u = '\v'; continue; }
            "\\\\"               { u = '\\'; continue; }
            "\\'"                { u = '\''; continue; }
            "\\\""               { u = '"';  continue; }
            "\\?"                { u = '?';  continue; }
        */
    }
    printf("%c", q);
    return 1;
}

int php_request_digest_lex(const char * str, size_t len)
{
    struct lex_ctx _in = {
        str,
        str,
        str,
        0,
        str + len
    };
    struct lex_ctx * in = &_in;

    for (;;) {
        in->tok = in->cur;
        /*!re2c
            re2c:define:YYCURSOR = in->cur;
            re2c:define:YYMARKER = in->mar;
            re2c:define:YYLIMIT = in->lim;
            re2c:yyfill:enable = 0;

            end = "\x00";

            *   { return 0; printf("sigh %c\n", *in->tok); continue; return 0; }
            end { return in->lim == in->tok; }

            // whitespaces
            mcm = "/*" ([^*] | ("*" [^/]))* "*""/";
            scm = "//" [^\n]* "\n";
            wsp = ([ \t\v\n\r] | scm | mcm)+;
            wsp { printf(" "); continue; }

            // character and string literals
            "L"? ['"] { if (!lex_str(in, *(in->cur - 1))) return 0; continue; }
            "L"? "''" { return 0; }

            // keywords
            "nonce"              { printf("nonce");              continue; }
            "nc"              { printf("nc");              continue; }
            "cnonce"              { printf("cnonce");              continue; }
            "qop"              { printf("qop");              continue; }
            "username"              { printf("username");              continue; }
            "uri"              { printf("uri");              continue; }
            "response"              { printf("response");              continue; }

            "="               { printf("=");      continue; }
            ","     { printf(",");     continue; }

            // identifiers
            id = [a-zA-Z_][a-zA-Z_0-9]*;
            id { printf("%.*s", in->cur - in->tok, in->tok); continue; }

        */
    }
}
