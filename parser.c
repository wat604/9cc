#include "9cc.h"

//
// grammer
//

// program = function*
// function = ident "(" args ")" "{" stmt* "}"
// stmt       = expr_stmt 
        // | "{" stmt* "}"
        // | "if" "(" expr ")" stmt ("else" stmt)?
        // | "while" "(" expr ")" stmt
        // | "for" "(" expr? ";" expr? ";" expr? ")" stmt
        // | "return" ";"
// expr_stmt  = expr ";"
// expr       = assign
// assign     = equality ("=" assign)?
// equality   = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// add        = mul ("+" mul | "-" mul)*
// mul        = unary ("*" unary | "/" unary)*
// unary      = ("+" | "-")? primary
// primary    = num 
//              | ident ("(" args? ")")? 
//              | "(" expr ")"
// args       = expr ("," expr)*

LVar *locals;

Node *code[100];

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

// local variable
LVar *new_lvar(char *name, int len) {
    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->name = name;
    lvar->len = len;
    lvar->offset = locals->offset + 8;
    lvar->next = locals;
    locals = lvar;
    return lvar;
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

Token *consume_ident() {
    if (token->kind != TK_IDENT)
        return false;
    Token *current_token = token;
    token = token->next;
    return current_token;
}

bool consume_kind(TokenKind kind) {
    if (token->kind != kind)
        return false;
    token = token->next;
    return true;
}

// 次のトークンが期待している記号の時には、トークンを１つ読み進める。
// それ以外の場合にはエラーを報告する。
// consumeは来るかどうかわからないとき。ifの条件式で使う
// expectは必ず来るとわかっているとき。トークンを進める＆文法チェック
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

void expect_kind(TokenKind kind, char *keyword) {
    if (token->kind != kind) {
        error_at(token->str, "'%s'ではありません。", keyword);
    }
    token = token->next;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

// 変数を名前で検索する。見つからなかった場合はNULLを返す。
LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var; var = var->next)
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
            return var;
    return NULL;
}


//////////////////////////////////
// AST grammers
//////////////////////////////////

// ASTを作るときに文法の左辺ごとにfunctionを作る
// functionはNodeを返す

Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *rational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

void program() {
    // Node *node;
    // Token *tok_fname = consume_ident();
    // node = new_node(ND_FUNCTION);
    // node->str = tok_fname->str;
    // node->len = tok_fname->len;

    // expect("(");

    LVar head;
    head.next = NULL;
    head.offset = 0;
    locals = &head;

    int i = 0;
    while (!at_eof()) {
        code[i++] = stmt();
    }
    code[i] = NULL;
}

Node *stmt() {
    Node *node;

    if (consume_kind(TK_IF)) {
        node = new_node(ND_IF);
        expect("(");
        node->cond = expr();
        expect(")");
        node->lhs = stmt();

        // 次のstmtを読んでelseかどうか判定する
        // 今のところstmtは1文で終わる前提
        if(consume_kind(TK_ELSE)) {
            node->kind = ND_IF_ELSE;
            node->els = stmt();
        }
        return node;
    }

    if (consume_kind(TK_WHILE)) {
        node = new_node(ND_WHILE);
        expect("(");
        node->cond = expr();
        expect(")");
        node->lhs = stmt();
        return node;
    }

    if (consume_kind(TK_FOR)) {
        node = new_node(ND_FOR);
        expect("(");
        if (consume(";") == false) {
            node->init = expr();
            expect(";");
        }
        if (consume(";") == false) {
            node->cond = expr();
            expect(";");
        }
        if (consume(")") == false) {
            node->inc = expr();
            expect(")");
        }
        node->lhs = stmt();
        return node;
    }

    // block
    if (consume("{")) {
        node = new_node(ND_BLOCK);
        Node *cur = node;
        while (consume("}") == false) {
            cur->block = stmt();
            cur = cur->block;
        } 
        return node;
    }

    if (consume_kind(TK_RETURN)) {
        node = new_node(ND_RETURN);
        node->lhs = expr();
    } else {
        node = new_node(ND_EXPR_STMT);
        node->lhs = expr();
    }

    expect(";");
    return node;
}

Node *expr() {
    Node *node = assign();
    return node;
}

Node *assign() {
    Node *node = equality();
    // 右辺がある場合は右辺が深くなるようになっている
    if(consume("="))
        node = new_binary(ND_ASSIGN, node, assign());
    return node;
}

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

Node *unary() {
    if (consume("+"))
        return primary();
    if (consume("-"))
        return new_binary(ND_SUB, new_num(0), primary());
    return primary();
}

Node *primary() {
    // 次のトークンが"("なら、 "(" expr ")"のはず
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok_ident = consume_ident();
    if (tok_ident) {
        // 関数呼び出しの場合
        if (consume("(")) {
            Node *node = new_node(ND_CALL);
            node->str = tok_ident->str;
            node->len = tok_ident->len;
            int num_args = 0;
            // 引数(arguments)ありの場合
            if (consume(")") == false) {
                Arg *head = calloc(1, sizeof(Arg));
                Arg *arg = head;
                arg->cur = expr();
                num_args++;
                while (consume(",")) {
                    arg->next = calloc(1, sizeof(Arg));
                    arg = arg->next;
                    arg->cur = expr();
                    num_args++;
                }
                node->args = head;
                node->num_args = num_args;
                expect(")");
            }
            return node;
        } else {
        // 変数の場合
            Node *node = new_node(ND_LVAR);

            LVar *lvar = find_lvar(tok_ident);
            if (lvar) {
                node->offset = lvar->offset;
            } else {
                lvar = new_lvar(tok_ident->str, tok_ident->len);
                node->offset = lvar->offset;
            }
            return node;
        }

    }

    // そうでなければ数値のはず
    return new_num(expect_number());
}

