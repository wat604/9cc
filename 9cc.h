#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TokenKind型の作成
typedef enum {
    TK_RESERVED,    // 記号
    TK_IDENT,       // 識別子
    TK_NUM,         // 整数トークン
    TK_RETURN,      // return
    TK_IF,          // if
    TK_ELSE,        // else
    TK_WHILE,       // while
    TK_FOR,         // for
    TK_EOF,         // 入力の終わりを表すトークン
} TokenKind;

// 抽象構文木のノードの種類
typedef enum {
    ND_ADD,     // +
    ND_SUB,     // -
    ND_MUL,     // *
    ND_DIV,     // /
    ND_EQ,      // ==
    ND_NE,      // !=
    ND_LT,      // <
    ND_LE,      // <=
    ND_ASSIGN,  // =
    ND_LVAR,    // ローカル変数
    ND_NUM,     // 整数
    ND_EXPR_STMT, // expr
    ND_RETURN,  // return
    ND_IF,      // if
    ND_IF_ELSE, // if ... else
    ND_ELSE,    // else
    ND_WHILE,   // while
    ND_FOR,     // for
    ND_BLOCK,   // { ... }
    ND_CALL,    // func()
    ND_FUNCTION // function difinition
} NodeKind;

// 変数定義の時に
// struct Token t = ...;
// が↓のように書ける
// Token t = ...;

// トークン型
typedef struct Token Token;
typedef struct Node Node;
typedef struct Arg Arg;

struct Token {
    TokenKind kind; // トークンの型
    Token *next;    // 次の入力トークン
    int val;        // kindがTK_NUMの場合、その数値
    char *str;      // トークンの文字列
    int len;        // トークンの文字列の長さ
};

// 抽象構文木のNode型
struct Node {
    NodeKind kind;  // ノードの型
    Node *lhs;
    Node *rhs;
    int val;        // kindがND_NUMの場合のみ使う
    int offset;     // kindがND_LVARの場合のみ使う

    // "if", "while" and "for" statement
    Node *cond;
    Node *init;
    Node *inc;
    Node *els;

    // blockの中のstmt*をリストで繋いでおく
    Node *block;

    // call function
    char *str;      // call function用の関数名
    int len;        // 関数名の長さ
    int num_args;       // 引数の数
    Arg *args;     // func(arg1, arg2, ...);

};

// function callの時の引数をリストで保持してしておく
struct Arg {
    Arg *next;
    Node *cur;
};

// プロトタイプ宣言
void program();


Token *tokenize(char *user_input);
void gen_stmt(Node *node);
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);

// グローバル変数の宣言
// 現在着目しているトークン
extern Token *token;

// stmt nodeを保存しておくグローバル変数
extern Node *code[100];

typedef struct LVar LVar;
struct LVar {
    LVar *next; // 次の変数かNULL
    char *name; // 変数の名前
    int len;    // 名前の長さ
    int offset; // RBPからのオフセット
};

// ローカル変数 連結リストの先頭のポインタ
extern LVar *locals;