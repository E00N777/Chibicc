#pragma once
#include "astnode.h"
#include "tokenize.h"
#include <string_view>


class Parser {
private:
    Token* current_;
    Obj* locals = nullptr;  // All local variables created during parsing (linked list).

    //=================================== Generic AST node construction ===================================
    static Node* new_binary(NodeKind kind, Node* lhs, Node* rhs, Token* tok);
    static Node* make_binary(NodeKind kind, Node* lhs, Node* rhs, Token* op_tok);
    static Node* make_func_call(NodeKind kind,Token* op_tok,std::string_view func_name);

    //=================================== Expression parsing (precedence-climbing) ===================================
    Node* primary();    // (expr) | ident | num
    Node* mul();        // * /
    Node* expr();       // top-level entry
    Node* unary();      // + - * &
    Node* equality();   // == !=
    Node* relational(); // >= > <= <
    Node* add();        // + -
    Node* expr_stmt();  // expr; or ;
    Node* stmt();       // return | while | for | if | block | expr_stmt
    Node* assign();     // = (right-associative)
    Node* compound_stmt();  // { stmt... }

    //=================================== Variable and local symbol management ===================================
    Obj* find_var(Token* tok);
    static Node* new_var_node(Obj* var, Token* tok);
    Obj* new_lvar(const std::string& name,Type* ty);

    //=================================== Pointer arithmetic (typed add/sub) ===================================
    static Node* new_num(int val, Token* tok);
    Node* new_add(Node* lhs, Node* rhs, Token* tok);
    Node* new_sub(Node* lhs, Node* rhs, Token* tok);

    //=================================== Declaration (reserved for future use) ===================================
    Type* declspec();
    std::pair<Type*,Token*> declarator(Type* basety);
    Node* declaration();

public:
    explicit Parser(Token* tk) : current_(tk) {}

    //=================================== Entry point ===================================
    Function* parse();

    //=================================== Token consumption (do not use tokenize.h directly here) ===================================
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

    /// Returns the content of the current (next to consume) token.
    std::string_view peek_content() const {
        return current_ ? current_->get_content() : std::string_view{};
    }

    /// Returns true if the token after the current one has the given content (e.g. ident followed by "(").
    bool is_followed_by(const char* str) const {
        Token* next = current_ ? current_->get_next() : nullptr;
        return next && Tkequal(next, str);
    }

    void advance() { current_ = current_->get_next(); }
    //=================================== End of Token consumption ===================================
};
