#include "9cc.h"

void gen_lval(Node *node) {
    if (node->kind != ND_LVAR)
        error("代入の左辺値が変数ではありません");

    // ベースポインタの値をraxに移す
    // 変数のアドレスをrax-offsetで計算
    // スタックトップに変数のアドレスをプッシュ
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n");
}

void gen(Node *node) {
    switch (node->kind) {
    case ND_NUM:
        printf("    push %d\n", node->val);
        return;
    case ND_LVAR:
        gen_lval(node);
        // スタックトップに変数のアドレスの値をプッシュ
        // 変数の値を展開してる
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    case ND_ASSIGN:
        // 左辺の変数のアドレスをプッシュ
        // 左辺がLVARでなかった場合のエラー出力もここで
        gen_lval(node->lhs);
        // 右辺を評価
        gen(node->rhs);

        printf("    pop rdi\n");
        printf("    pop rax\n");
        // 左辺の変数のアドレスが示す場所に右辺の結果を移動
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
        return;
    }


    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n"); // rhsの結果をpop
    printf("    pop rax\n"); // lhsの結果をpop

    switch (node->kind) {
    case ND_ADD:
        printf("    add rax, rdi\n");
        break;
    case ND_SUB:
        printf("    sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("    imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("    cqo\n"); // raxを128bitにセット
        printf("    idiv rdi\n"); // rax / rdi
        break;
    case ND_EQ:
        printf("    cmp rax, rdi\n");
        printf("    sete al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_NE:
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LT:
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LE:
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;
    }

    printf("    push rax\n");
}
