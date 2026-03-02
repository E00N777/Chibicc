#include "parser.h"
#include "astnode.h"
#include "tokenize.h"
#include "diagnostic.h"
#include "type.h"
#include <utility>


// Find a local variable by name.
Obj* Parser::find_var(Token* tok) {
    std::string_view name = tok->get_content();
    for (Obj* var = locals; var; var = var->get_next())
        if (var->get_name() == name)
            return var;
    return nullptr;
}

Node* Parser::new_var_node(Obj* var, Token* tok) {
    Node* node = new Node(NodeKind::ND_VAR, var);
    node->set_tok(tok);
    return node;
}

Obj* Parser::new_lvar(const std::string& name) {
    Obj* var = new Obj(name, locals);
    locals = var;
    return var;
}

Node* Parser::new_binary(NodeKind kind, Node* lhs, Node* rhs, Token* tok) {
    Node* node = new Node(kind, lhs, rhs);
    node->set_tok(tok);
    return node;
}

Node* Parser::make_binary(NodeKind kind, Node* lhs, Node* rhs, Token* op_tok) {
    return new_binary(kind, lhs, rhs, op_tok);
}

Node* Parser::new_num(int val, Token* tok) {
    Node* node = new Node(val);
    node->set_tok(tok);
    return node;
}

// int+int => add; ptr+int or int+ptr => scale int by 8 then add.
Node* Parser::new_add(Node* lhs, Node* rhs, Token* tok) {
    add_type(lhs);
    add_type(rhs);

    if (is_integer(lhs->get_ty()) && is_integer(rhs->get_ty()))
        return new_binary(NodeKind::ND_ADD, lhs, rhs, tok);

    if (lhs->get_ty()->get_base() && rhs->get_ty()->get_base())
        diagnostic::error_tok(tok, "invalid operands");

    // Canonicalize num+ptr => ptr+num.
    if (!lhs->get_ty()->get_base() && rhs->get_ty()->get_base())
        std::swap(lhs, rhs);

    // ptr + num: scale rhs by 8 (element size).
    rhs = new_binary(NodeKind::ND_MUL, rhs, new_num(8, tok), tok);
    return new_binary(NodeKind::ND_ADD, lhs, rhs, tok);
}


// int-int => sub; ptr-int => scale then sub; ptr-ptr => byte diff / 8.
Node* Parser::new_sub(Node* lhs, Node* rhs, Token* tok) {
    add_type(lhs);
    add_type(rhs);

    if (is_integer(lhs->get_ty()) && is_integer(rhs->get_ty()))
        return new_binary(NodeKind::ND_SUB, lhs, rhs, tok);

    if (lhs->get_ty()->get_base() && is_integer(rhs->get_ty())) {
        rhs = new_binary(NodeKind::ND_MUL, rhs, new_num(8, tok), tok);
        add_type(rhs);
        Node* node = new_binary(NodeKind::ND_SUB, lhs, rhs, tok);
        node->set_ty(lhs->get_ty());
        return node;
    }

    if (lhs->get_ty()->get_base() && rhs->get_ty()->get_base()) {
        Node* node = new_binary(NodeKind::ND_SUB, lhs, rhs, tok);
        node->set_ty(get_ty_int());
        return new_binary(NodeKind::ND_DIV, node, new_num(8, tok), tok);
    }

    diagnostic::error_tok(tok, "invalid operands");
}

Node* Parser::compound_stmt()
{
    Token* block_tok = peek();
    Node head(NodeKind::ND_EXPR_STMT);  // sentinel for statement list
    Node* cur = &head;
    while (!check("}")) {
        Node* stmt_node = stmt();
        cur->set_nextstmt(stmt_node);
        cur = cur->get_nextstmt();
        add_type(cur);
    }
    Node* node = new Node(NodeKind::ND_BLOCK);
    node->set_tok(block_tok);
    node->set_body(head.get_nextstmt());
    expect("}");
    return node;
}

Function* Parser::parse() {
    expect("{");
    Function* prog = new Function();
    prog->set_body(compound_stmt());
    prog->set_locals(locals);
    return prog;
}

Node* Parser::stmt()
{
    if (check("return")) {
        Token* ret_tok = peek();
        consume("return");
        Node* node = new Node(NodeKind::ND_RETURN, expr());
        node->set_tok(ret_tok);
        expect(";");
        return node;
    }

    if (check("while")) {
        Token* while_tok = peek();
        consume("while");
        Node* node = new Node(NodeKind::ND_FOR);
        node->set_tok(while_tok);
        expect("(");
        node->set_condition(expr());
        expect(")");
        node->set_then(stmt());
        return node;
    }

    if (check("for")) {
        Token* for_tok = peek();
        consume("for");
        Node* node = new Node(NodeKind::ND_FOR);
        node->set_tok(for_tok);
        expect("(");
        node->set_init(expr_stmt());
        if (!check(";"))
            node->set_condition(expr());
        expect(";");
        if (!check(")"))
            node->set_inc(expr());
        expect(")");
        node->set_then(stmt());
        return node;
    }

    if (check("if")) {
        Token* if_tok = peek();
        consume("if");
        Node* node = new Node(NodeKind::ND_IF);
        node->set_tok(if_tok);
        expect("(");
        node->set_condition(expr());
        expect(")");
        node->set_then(stmt());
        if (consume("else"))
            node->set_els(stmt());
        return node;
    }

    if (consume("{")) {
        Node* node = compound_stmt();
        return node;
    }

    return expr_stmt();
}

Node* Parser::expr_stmt()
{
    Token* stmt_tok = peek();
    if (consume(";")) {
        Node* node = new Node(NodeKind::ND_BLOCK);
        node->set_tok(stmt_tok);
        return node;
    }
    Node* node = new Node(NodeKind::ND_EXPR_STMT, expr());
    node->set_tok(stmt_tok);
    expect(";");
    return node;
}


Node* Parser::assign()
{
    Node* node = equality();

    if (check("=")) {
        Token* assign_tok = peek();
        consume("=");
        Node* rhs = assign();
        node = make_binary(NodeKind::ND_ASSIGN, node, rhs, assign_tok);
    }
    return node;
}

//lowest level
Node* Parser::expr()
{
    return assign();
}

Node* Parser::equality()
{
    Node* node = relational();

    for (;;) {
        if (check("==")) {
            Token* op_tok = peek();
            consume("==");
            node = make_binary(NodeKind::ND_EQ, node, relational(), op_tok);
            continue;
        }
        if (check("!=")) {
            Token* op_tok = peek();
            consume("!=");
            node = make_binary(NodeKind::ND_NE, node, relational(), op_tok);
            continue;
        }
        return node;
    }
}

Node* Parser::relational()
{
    Node* node = add();
    for (;;) {
        if (check(">=")) {
            Token* op_tok = peek();
            consume(">=");
            node = make_binary(NodeKind::ND_LE, add(), node, op_tok);
            continue;
        }
        if (check(">")) {
            Token* op_tok = peek();
            consume(">");
            node = make_binary(NodeKind::ND_LT, add(), node, op_tok);
            continue;
        }
        if (check("<=")) {
            Token* op_tok = peek();
            consume("<=");
            node = make_binary(NodeKind::ND_LE, node, add(), op_tok);
            continue;
        }
        if (check("<")) {
            Token* op_tok = peek();
            consume("<");
            node = make_binary(NodeKind::ND_LT, node, add(), op_tok);
            continue;
        }
        return node;
    }
}


Node* Parser::add() {
    Node* node = mul();

    for (;;) {
        if (check("+")) {
            Token* op_tok = peek();
            consume("+");
            node = new_add(node, mul(), op_tok);
            continue;
        }
        if (check("-")) {
            Token* op_tok = peek();
            consume("-");
            node = new_sub(node, mul(), op_tok);
            continue;
        }
        return node;
    }
}

// Mid level
Node* Parser::mul()
{
    Node* node = unary();
    for (;;) {
        if (check("*")) {
            Token* op_tok = peek();
            consume("*");
            node = make_binary(NodeKind::ND_MUL, node, unary(), op_tok);
            continue;
        }
        if (check("/")) {
            Token* op_tok = peek();
            consume("/");
            node = make_binary(NodeKind::ND_DIV, node, unary(), op_tok);
            continue;
        }
        return node;
    }
}

// Top level
Node* Parser::primary()
{
    if (consume("(")) {
        Node* node = expr();
        expect(")");
        return node;
    }
    if (peek()->get_kind() == TokenKind::IDENT) {
        Token* ident_tok = peek();
        Obj* var = find_var(peek());
        if (!var)
            var = new_lvar(std::string(peek()->get_content()));
        advance();
        return new_var_node(var, ident_tok);
    }
    if (peek()->get_kind() == TokenKind::NUM) {
        Token* num_tok = peek();
        Node* node = new Node(peek()->get_number());
        node->set_tok(num_tok);
        advance();
        return node;
    }
    diagnostic::error_at(peek()->get_content(), "expected an expression");
}   

// unary = ("+" | "-" | "*" | "&") unary
Node* Parser::unary()
{
    if (consume("+"))
        return unary();
    if (check("-")) {
        Token* minus_tok = peek();
        consume("-");
        Node* node = new Node(NodeKind::ND_NEG, unary());
        node->set_tok(minus_tok);
        return node;
    }
    if (check("&")) {
        Token* addr_tok = peek();
        consume("&");
        Node* node = new Node(NodeKind::ND_ADDR, unary());
        node->set_tok(addr_tok);
        return node;
    }
    if (check("*")) {
        Token* deref_tok = peek();
        consume("*");
        Node* node = new Node(NodeKind::ND_DEREF, unary());
        node->set_tok(deref_tok);
        return node;
    }
    return primary();
}