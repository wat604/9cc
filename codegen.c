#include "9cc.h"

int label_index = 0;

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
    // 数値の場合はここで処理。
    // LHSとRHSは存在しないのでreturn。
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
    case ND_RETURN:
        gen(node->lhs);
        printf("    pop rax\n");
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
        return;
    case ND_IF:
        int current_label_index_if = label_index++;
        gen(node->lhs);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%d\n", current_label_index_if);
        gen(node->rhs);
        printf(".Lend%d:\n", current_label_index_if);
        return;
    case ND_IF_ELSE:
        int current_label_index = label_index++;
        gen(node->lhs);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lelse%d\n", current_label_index);
        // elseのNodeは直接飛ばして読む
        gen(node->rhs->lhs);
        printf("    je .Lend%d\n", current_label_index);
        printf(".Lelse%d:\n", current_label_index);
        gen(node->rhs->rhs);
        printf(".Lend%d:\n", current_label_index);
        return;
    }

    // 以下exprの演算処理の複数項のため再帰している
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
