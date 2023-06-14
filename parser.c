#include "9cc.h"

Node *new_node(NodeKind kind) {
    Node *node = calloc(1, sizeof(Node)); // callocはメモリの確保と0で初期化。mallocは初期化がない。
    node->kind = kind;
    return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_num(int val) {
    Node *node = new_node(ND_NUM);
    node->val = val;
    return node;
}



// 次のトークンが期待している記号の時には、トークンを１つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
    if (token->kind != TK_RESERVED || 
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        return false;
    token = token->next; 
    return true;
}

// 次のトークンが期待している記号の時には、トークンを１つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op) {
    if (token->kind != TK_RESERVED || 
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        error_at(token->str, "'%s'ではありません", op);
    token = token->next; 
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
    if (token->kind != TK_NUM)
        error_at(token->str, "数ではありません。");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
}


//////////////////////////////////
// AST grammers
//////////////////////////////////

// ASTを作るときに文法の左辺ごとにfunctionを作る
// functionはNodeを返す


Node *expr() {
    Node *node = equality();
    return node;
}

// equality   = relational ("==" relational | "!=" relational)*
Node *equality() {
    Node *node = rational();

    for (;;) {
        if (consume("=="))
            node = new_binary(ND_EQ, node, rational());
        else if (consume("!="))
            node = new_binary(ND_NE, node, rational());
        else
            return node;
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *rational() {
    Node *node = add();

    for (;;) {
        if (consume("<="))
            node = new_binary(ND_LE, node, add());
        else if (consume("<"))
            node = new_binary(ND_LT, node, add());
        else if (consume(">="))
            node = new_binary(ND_LE, add(), node);
        else if (consume(">"))
            node = new_binary(ND_LT, add(), node);
        else
            return node;
    } 
}

// add = mul ( '+' mul | '-' mul )*
Node *add() {
    Node *node = mul();

    for (;;) {
        if (consume("+"))
            node = new_binary(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_binary(ND_SUB, node, mul());
        else
            return node;
    }
}

// mul = unary ( '*' unary | '/' unary )*
Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume("*"))
            node = new_binary(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_binary(ND_DIV, node, unary());
        else
            return node;
    }
}

// unary = ("+" | "-")? primary
Node *unary() {
    if (consume("+"))
        return primary();
    if (consume("-"))
        return new_binary(ND_SUB, new_num(0), primary());
    return primary();
}

// primary = num | '(' primary ')'
Node *primary() {
    // 次のトークンが"("なら、 "(" expr ")"のはず
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    // そうでなければ数値のはず
    return new_num(expect_number());
}
