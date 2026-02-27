#pragma once
#include "astnode.h"
#include "tokenize.h"

class Parser {
private:
    Token* current;
    Obj* locals = nullptr;  // all local variables created during parsing

    Node* primary();
    Node* mul();
    Node* expr();
    Node* unary();
    Node* equality();
    Node* relational();
    Node* add();
    Node* expr_stmt();
    Node* stmt();
    Node* assign();
    Node* compound_stmt();

    Obj* find_var(Token* tok);
    static Node* new_var_node(Obj* var, Token* tok);
    Obj* new_lvar(const std::string& name);

    // Helpers for typed arithmetic (pointer + int scales by element size).
    static Node* new_binary(NodeKind kind, Node* lhs, Node* rhs, Token* tok);
    static Node* new_num(int val, Token* tok);
    Node* new_add(Node* lhs, Node* rhs, Token* tok);
    Node* new_sub(Node* lhs, Node* rhs, Token* tok);

public:
    Parser(Token* tk) : current(tk) {}

    Function* parse();
};
