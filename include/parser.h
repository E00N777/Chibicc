#pragma once
#include "astnode.h"
#include "tokenize.h"

class Parser {
private:
    Token* current_;
    Obj* locals = nullptr;  // all local variables created during parsing
    
private:
    static Node* new_binary(NodeKind kind, Node* lhs, Node* rhs, Token* tok);
    static Node* make_binary(NodeKind kind, Node* lhs, Node* rhs, Token* op_tok);

    // Helper functions for parsing
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
    static Node* new_num(int val, Token* tok);
    Node* new_add(Node* lhs, Node* rhs, Token* tok);
    Node* new_sub(Node* lhs, Node* rhs, Token* tok);

public:
    explicit Parser(Token* tk) : current_(tk) {}

    //---Entry point for parsing---
    Function* parse();

    //func for consuming tokens 
    //Don't use Tokenize.h functions directly here
    bool consume(const char* op )
    {
        if(Tkequal(current_, op))
        {
            current_ = current_->get_next();
            return true;
        }
        return false;
    }

    void expect(const char* op)
    {
        Tkskip(current_, op);
    }

    bool check(const char* op) const {
        return Tkequal(current_, op);
    }

    Token* peek() const { return current_; }

    void advance() { current_ = current_->get_next(); }
};
