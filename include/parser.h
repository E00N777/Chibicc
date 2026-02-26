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

public:
    Parser(Token* tk) : current(tk) {}

    Function* parse();
};
