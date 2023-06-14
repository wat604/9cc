#include "9cc.h"

// グローバル変数 tokenの定義
Token *token;

int main(int argc, char **argv) {
    if (argc != 2) {
        // argv[0]がコンパイラ、argv[1]が入力するコードに相当
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    // トークナイズする
    token = tokenize(argv[1]);
    Node *node = expr();

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // 抽象構文木を下りながらコード生成
    gen(node);

    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}
