#pragma once
#include <string>
#include <string_view>

// Local variable: name and stack offset from RBP.
struct Obj {
    Obj* next = nullptr;
    std::string name;
    int offset = 0;
};

// Function: body statements and list of local variables.
struct Function {
    class Node* body = nullptr;
    Obj* locals = nullptr;
    int stack_size = 0;
};

// Node for AST
enum class NodeKind
{
    ND_ADD,       // +
    ND_SUB,       // -
    ND_MUL,       // *
    ND_DIV,       // /
    ND_NUM,       // integer
    ND_NEG,       // unary minus
    ND_EQ,        // ==
    ND_NE,        // !=
    ND_LT,        // <
    ND_LE,        // <=
    ND_EXPR_STMT, // expression statement
    ND_ASSIGN,    // assignment
    ND_VAR,       // variable (uses var pointer)
};

class Node {
private:
    NodeKind kind;
    Node* lhs = nullptr;
    Node* rhs = nullptr;
    Node* next = nullptr;
    int val = 0;
    Obj* var = nullptr;  // used when kind == ND_VAR

public:
    Node(NodeKind kind) : kind(kind) {}
    Node(NodeKind kind, Node* lhs, Node* rhs) : kind(kind), lhs(lhs), rhs(rhs) {}
    Node(NodeKind kind, Node* lhs) : kind(kind), lhs(lhs) {}
    Node(int val) : val(val) { this->kind = NodeKind::ND_NUM; }
    Node(NodeKind kind, Obj* var) : kind(kind), var(var) {}

    NodeKind get_nodekind() const { return kind; }
    int get_val() const { return val; }
    Node* get_lhs() const { return lhs; }
    Node* get_rhs() const { return rhs; }
    Node* get_nextstmt() const { return next; }
    Obj* get_var() const { return var; }

    void set_nextstmt(Node* n) { next = n; }
    void set_var(Obj* v) { var = v; }
};