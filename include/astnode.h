#pragma once
#include <string>
#include <string_view>

// Forward declarations.
class Node;
class Token;
class Type;

// --- AST and symbol representation ---
// Obj: one local variable (name, type, stack offset). Linked via next_ per function.
// Function: one function (body AST, locals list, name). Linked via next_ for the program.
// Node: AST node; meaning of lhs/rhs/body/condition/etc. depends on NodeKind.

// Local variable: name, type, and stack offset from RBP (filled during codegen).
class Obj {
public:
    explicit Obj(std::string name, Obj* next = nullptr)
        : next_(next), name_(std::move(name)), offset_(0) {}
    explicit Obj(std::string name, Type* ty, Obj* next = nullptr)
        : next_(next), name_(std::move(name)), offset_(0), ty_(ty) {}

    Obj* get_next() const { return next_; }
    void set_next(Obj* next) { next_ = next; }
    const std::string& get_name() const { return name_; }
    int get_offset() const { return offset_; }
    void set_offset(int offset) { offset_ = offset; }
    Type* get_ty() const { return ty_; }
    void set_ty(Type* ty) { ty_ = ty; }

private:
    Obj* next_;
    std::string name_;
    int offset_;
    Type* ty_ = nullptr;
};

// Function: body (first stmt of compound), locals list, and name. next_ chains all functions.
class Function {
public:
    Function() = default;

    Function(Node* body, Obj* locals, std::string_view name)
    : body_(body), locals_(locals), stack_size_(0), next_(nullptr), name_(name) {}

    Node* get_body() const { return body_; }
    void set_body(Node* body) { body_ = body; }
    Obj* get_locals() const { return locals_; }
    void set_locals(Obj* locals) { locals_ = locals; }
    int get_stack_size() const { return stack_size_; }
    void set_stack_size(int size) { stack_size_ = size; }
    
    Function* get_next() const { return next_; }
    void set_next(Function* fn) { next_ = fn; }
    std::string_view get_name() const { return name_; }
    void set_name(std::string_view n) { name_ = n; }

private:
    Node* body_ = nullptr;
    Obj* locals_ = nullptr;
    int stack_size_ = 0;
    Function* next_=nullptr;
    std::string_view name_;
};

// AST node kind. Which of lhs/rhs/body/condition/then/els/init/inc/var/args is valid depends on kind.
enum class NodeKind {
    ND_ADD, ND_SUB, ND_MUL, ND_DIV, ND_NUM, ND_NEG,
    ND_EQ, ND_NE, ND_LT, ND_LE,
    ND_EXPR_STMT, ND_ASSIGN, ND_VAR, ND_RETURN, ND_BLOCK,
    ND_IF, ND_FOR, ND_ADDR, ND_DEREF, ND_FUNCALL,
};

class Node {
private:
    NodeKind kind;
    Node* lhs = nullptr;
    Node* rhs = nullptr;
    Node* next = nullptr;
    Token* tok = nullptr;   // For error reporting.
    int val = 0;            // ND_NUM only.
    Obj* var = nullptr;    // ND_VAR only.
    Node* body = nullptr;   // ND_BLOCK, ND_EXPR_STMT (as stmt list), etc.
    Node* condition = nullptr;  // ND_IF, ND_FOR.
    Node* then = nullptr;
    Node* els = nullptr;
    Node* init = nullptr;
    Node* inc = nullptr;

    Type* ty = nullptr;         // Filled by add_type() (type.cpp); used by codegen and parser (+/-).
    std::string_view func_name; // ND_FUNCALL: callee name.
    Node* args = nullptr;       // ND_FUNCALL: first argument (chain via get_nextstmt()).

public:
    Node(NodeKind kind) : kind(kind) {}
    Node(NodeKind kind, Node* lhs, Node* rhs) : kind(kind), lhs(lhs), rhs(rhs) {}
    Node(NodeKind kind, Node* lhs) : kind(kind), lhs(lhs) {}
    Node(int val) : val(val) { this->kind = NodeKind::ND_NUM; }
    Node(NodeKind kind, Obj* var) : kind(kind), var(var) {}

    NodeKind get_nodekind() const { return kind; }
    Token* get_tok() const { return tok; }
    void set_tok(Token* t) { tok = t; }
    Type* get_ty() const { return ty; }
    void set_ty(Type* t) { ty = t; }
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

    void set_func_name(std::string_view name) { func_name = name; }
    std::string_view get_func_name() const { return func_name; }
    Node* get_args() const { return args; }
    void set_args(Node* a) { args = a; }
};