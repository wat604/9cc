#include "9cc.h"

int label_index = 0;

static void gen_expr(Node *node);

static void gen_lval(Node *node) {
    if (node->kind != ND_LVAR)
        error("代入の左辺値が変数ではありません");

    // ベースポインタの値をraxに移す
    // 変数のアドレスをrax-offsetで計算
    // スタックトップに変数のアドレスをプッシュ
    printf("    # push an address of lvar\n");
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n");
}

static void gen_args(int argc, Node *args) {
        printf("    # copy args to registors\n");
        for (Node *cur = args; cur; cur = cur->args) {
            gen_expr(cur);
        }
        if (argc >= 6)
            printf("    pop r9\n");
        if (argc >= 5)
            printf("    pop r8\n");
        if (argc >= 4)
            printf("    pop rcx\n");
        if (argc >= 3)
            printf("    pop rdx\n");
        if (argc >= 2)
            printf("    pop rsi\n");
        if (argc >= 1)
            printf("    pop rdi\n");

        return;
}

void gen_stmt(Node *node) {
    int current_label_index;

    switch (node->kind) {
    case ND_EXPR_STMT:
        gen_expr(node->lhs);
        // 式の評価結果としてスタックに一つの値が残っている
        // はずなので、スタックが溢れないようにポップしておく
        printf("    pop rax\n");
        return;
    case ND_RETURN:
        printf("    # start return statement\n");
        gen_expr(node->lhs);
        printf("    pop rax\n");
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        // retはスタックをポップしてそのアドレスに飛ぶ
        // この時点でスタックトップは実行中の関数のリターンアドレス
        printf("    ret\n");
        return;
    case ND_IF:
        current_label_index = label_index++;
        printf("    # start if statement\n");
        printf("    # evaluate conditional expr\n");
        gen_expr(node->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%d\n", current_label_index);
        printf("    # true case%d\n", current_label_index);
        gen_stmt(node->lhs);
        printf(".Lend%d:\n", current_label_index);
        return;
    case ND_IF_ELSE:
        current_label_index = label_index++;
        printf("    # start if statement\n");
        printf("    # evaluate condition expr\n");
        gen_expr(node->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lelse%d\n", current_label_index);
        printf("    # true case%d\n", current_label_index);
        gen_stmt(node->lhs);
        printf("    jmp .Lend%d\n", current_label_index);
        printf(".Lelse%d:\n", current_label_index);
        printf("    # else case%d\n", current_label_index);
        gen_stmt(node->els);
        printf(".Lend%d:\n", current_label_index);
        return;
    case ND_WHILE:
        current_label_index = label_index++;
        printf("    # start while statement\n");
        printf("    # evaluate condition expr\n");
        printf(".Lbegin%d:\n", current_label_index);
        gen_expr(node->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%d\n", current_label_index);
        gen_stmt(node->lhs);
        printf("    jmp .Lbegin%d\n", current_label_index);
        printf(".Lend%d:\n", current_label_index);
        return;
    case ND_FOR:
        current_label_index = label_index++;
        printf("    # start for statement\n");
        if (node->init) {
            gen_expr(node->init);
            printf("    pop rax\n");
        }
        printf(".Lbegin%d:\n", current_label_index);
        if (node->cond) {
            gen_expr(node->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je .Lend%d\n", current_label_index);
        }
        gen_stmt(node->lhs);
        if (node->inc) {
            gen_expr(node->inc);
            printf("    pop rax\n");
        }
        printf("    jmp .Lbegin%d\n", current_label_index);
        printf(".Lend%d:\n", current_label_index);
        return;
    case ND_BLOCK:
        Node *cur;
        cur = node->block;
        while (cur) {
            gen_stmt(cur);
            cur = cur->block;
        }
        return;
    }
}

static void gen_expr(Node *node) {
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
        printf("    # push a value of lvar\n");
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    case ND_ASSIGN:
        printf("    # start assingnment\n");
        // 左辺の変数のアドレスをプッシュ
        // 左辺がLVARでなかった場合のエラー出力もここで
        gen_lval(node->lhs);
        // 右辺を評価
        gen_expr(node->rhs);

        printf("    pop rdi\n");
        printf("    pop rax\n");
        // 左辺の変数のアドレスが示す場所に右辺の結果を移動
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
        return;
    case ND_CALL:
        printf("    # call function\n");
        if (node->argc)
            gen_args(node->argc, node->args);

        // set RSP to 16x number
        printf("    mov rax, rsp\n"); // rax = rsp
        printf("    and rax, 8\n");   // rax = 8 & rax
        printf("    sub rsp, rax\n"); // rsp = rsp - 8 or rps
        printf("    push rax\n");     // push offset for rsp

        printf("    mov rax, %d\n", node->argc);
        printf("    call %.*s\n", node->len, node->str);
        // printf("    push rax\n");       // return value is in rax?
        printf("    pop rdi\n");     // pop the offset
        printf("    add rsp, rdi\n");// add the offset to rsp
        printf("    push rax\n");    // push return value

        return;
    }

    // 以下exprの演算処理の複数項のため再帰している
    gen_expr(node->lhs);
    gen_expr(node->rhs);

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
