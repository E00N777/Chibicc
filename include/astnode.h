#pragma once
#include <string>
#include <string_view>

// Forward declarations.
class Node;
class Token;

// Local variable: name and stack offset from RBP.
class Obj {
public:
    explicit Obj(std::string name, Obj* next = nullptr)
        : next_(next), name_(std::move(name)), offset_(0) {}

    Obj* get_next() const { return next_; }
    void set_next(Obj* next) { next_ = next; }
    const std::string& get_name() const { return name_; }
    int get_offset() const { return offset_; }
    void set_offset(int offset) { offset_ = offset; }

private:
    Obj* next_;
    std::string name_;
    int offset_;
};

// Function: body statements and list of local variables.
class Function {
public:
    Function() = default;

    Node* get_body() const { return body_; }
    void set_body(Node* body) { body_ = body; }
    Obj* get_locals() const { return locals_; }
    void set_locals(Obj* locals) { locals_ = locals; }
    int get_stack_size() const { return stack_size_; }
    void set_stack_size(int size) { stack_size_ = size; }

private:
    Node* body_ = nullptr;
    Obj* locals_ = nullptr;
    int stack_size_ = 0;
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
    ND_RETURN,    // return
    ND_BLOCK,     // block
    ND_IF,        // if
    ND_FOR,       // for and while statement
    ND_ADDR,      // address of
    ND_DEREF,     // dereference
};

class Node {
private:
    NodeKind kind;
    Node* lhs = nullptr;
    Node* rhs = nullptr;
    Node* next = nullptr;
    Token* tok = nullptr;  // Representative token
    int val = 0;
    Obj* var = nullptr;  // used when kind == ND_VAR
    Node* body = nullptr;
    //Support for if and for statement
    Node* condition = nullptr;
    Node* then = nullptr;
    Node* els = nullptr;
    Node* init = nullptr;
    Node* inc=nullptr;


public:
    Node(NodeKind kind) : kind(kind) {}
    Node(NodeKind kind, Node* lhs, Node* rhs) : kind(kind), lhs(lhs), rhs(rhs) {}
    Node(NodeKind kind, Node* lhs) : kind(kind), lhs(lhs) {}
    Node(int val) : val(val) { this->kind = NodeKind::ND_NUM; }
    Node(NodeKind kind, Obj* var) : kind(kind), var(var) {}

    NodeKind get_nodekind() const { return kind; }
    Token* get_tok() const { return tok; }
    void set_tok(Token* t) { tok = t; }
    int get_val() const { return val; }
    Node* get_lhs() const { return lhs; }
    Node* get_rhs() const { return rhs; }
    Node* get_nextstmt() const { return next; }
    Obj* get_var() const { return var; }

    void set_nextstmt(Node* n) { next = n; }
    void set_var(Obj* v) { var = v; }
    void set_body(Node* b) { body = b; }
    Node* get_body() const { return body; }

    void set_condition(Node* c) { condition = c; }
    void set_then(Node* t) { then = t; }
    void set_els(Node* e) { els = e; }
    Node* get_condition() const { return condition; }
    Node* get_then() const { return then; }
    Node* get_els() const { return els; }
    void set_init(Node* i) { init = i; }
    Node* get_init() const { return init; }
    void set_inc(Node* i) { inc = i; }
    Node* get_inc() const { return inc; }
};