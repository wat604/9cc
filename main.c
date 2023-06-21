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
    // コードの抽象構文木はグローバル変数codeに格納
    program();

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // プロローグ
    // 変数個分の領域を確保する
    printf("    # prologue\n");
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, %d\n", locals->offset);

    // 先頭の式から順にコード生成
    for (int i = 0; code[i]; i++) {
        gen_stmt(code[i]);
    }

    // エピローグ
    // 最後の式の結果がRAXに残っているのでそれが返り値になる
    printf("    # epilogue\n");
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
    return 0;
}
