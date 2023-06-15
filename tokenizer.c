#include "9cc.h"

static char *user_input;


static Token *new_token(TokenKind kind, Token *cur, char *str);

int is_alnum(char c) {
    return  ('a' <= c && c <= 'z') ||
            ('A' <= c && c <= 'Z') ||
            ('0' <= c && c <= '9') ||
            (c == '_'); 
}

// 入力文字列をトークナイズしてそれを返す
Token *tokenize(char *p) {
    user_input = p;
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        // 空白文字をスキップ
        if (isspace(*p)) {
            p++;
            continue;
        }

        // handle < <= > >=
        if (strchr("<>", *p)) {
            if (strchr("=", *(p + 1))) {
                cur = new_token(TK_RESERVED, cur, p);
                p = p + 2;
                cur->len = 2;                
            } else {
                cur = new_token(TK_RESERVED, cur, p++);
                cur->len = 1;
            }
            continue;
        }

        // handle == != =
        if (*p == '=') {
            if(*(p + 1) == '=') {
                cur = new_token(TK_RESERVED, cur, p);
                p = p + 2;
                cur->len = 2;
                continue;
            } else {
                cur = new_token(TK_RESERVED, cur, p++);
                cur->len = 1;
                continue;
            }
        }
        if (*p == '!') {
            cur = new_token(TK_RESERVED, cur, p);
            p = p + 2;
            cur->len = 2;
            continue;
        }

        if (strchr("+-*/();", *p)) {
            cur = new_token(TK_RESERVED, cur, p++); // p++の返り値は++する前のポインタ。charの値ではない。
            cur->len = 1;
            continue;
        }

        // return
        // returnで始まり、変数でない場合
        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
            cur = new_token(TK_RETURN, cur, p);
            cur->len = 6;
            p += 6;
            continue;
        }

        // variables
        // 文字列をトークンに追加
        // 変数の文字列は"a-zA-Z_"で始まり、先頭以外はそれに加え数値もOK
        if (('a' <= *p && *p <= 'z') ||
            ('A' <= *p && *p <= 'Z') ||
            (*p == '_')) {
            char *start;
            start = p;
            while (is_alnum(*p)) {
                p++;
            }
            cur = new_token(TK_IDENT, cur, start);
            cur->len = p - start;
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "トークナイズできません");

    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

// エラー箇所を報告する
// printfと同じ引数を取る 
// ...は可変長引数を表すCの文法。stdarg.hと一緒に使うのが一般的のようだ。
void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;

    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    fprintf(stderr, "%s\n", user_input);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// 新しいトークンを作成してcurに繋げる
static Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}
