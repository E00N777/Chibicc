#pragma once
#include <string>
#include <string_view>
#include <vector>

// Forward declarations.
class Node;
class Token;
class Type;
class Function;
class Program;
class GlobalVariable;


// --- AST and symbol representation ---
// Obj: one local variable (name, type, stack offset). Linked via next_ per function.
// Function: one function (body AST, locals list, name). Linked via next_ for the program.
// Node: AST node; meaning of lhs/rhs/body/condition/etc. depends on NodeKind.

// Local variable: name, type, and stack offset from RBP (filled during codegen).
class LocalVariable {
public:
    explicit LocalVariable(std::string name, LocalVariable* next = nullptr)
        : next_(next), name_(std::move(name)), offset_(0) {}
    explicit LocalVariable(std::string name, Type* ty, LocalVariable* next = nullptr)
        : next_(next), name_(std::move(name)), offset_(0), ty_(ty) {}

    LocalVariable* get_next() const { return next_; }
    void set_next(LocalVariable* next) { next_ = next; }
    const std::string& get_name() const { return name_; }
    int get_offset() const { return offset_; }
    void set_offset(int offset) { offset_ = offset; }
    Type* get_ty() const { return ty_; }
    void set_ty(Type* ty) { ty_ = ty; }

private:
    LocalVariable* next_ = nullptr;
    std::string name_;
    int offset_;
    Type* ty_ = nullptr;
};

// Global variable: name, type, and address.
class GlobalVariable {
    public:
        GlobalVariable(std::string_view name, Type* ty, GlobalVariable* next = nullptr)
            : next_(next), name_(std::move(name)), ty_(ty) {};
            
        GlobalVariable* get_next() const { return next_; }
        void set_next(GlobalVariable* next) { next_ = next; }
    
        const std::string_view& get_name() const { return name_; }
        Type* get_ty() const { return ty_; }

    private:
        GlobalVariable* next_ = nullptr;
        std::string_view name_;
        Type* ty_ = nullptr;
        
};

// Function: body (first stmt of compound), locals list, and name. next_ chains all functions.
// params_ stores the parameter objects in source order so codegen can map them to ABI
// argument registers without depending on the reversed locals linked list order.
class Function {
public:
    Function() = default;

    Function(Node* body, LocalVariable* locals, std::string_view name)
    : body_(body), locals_(locals), stack_size_(0), next_(nullptr), name_(name) {}

    Node* get_body() const { return body_; }
    void set_body(Node* body) { body_ = body; }
    LocalVariable* get_locals() const { return locals_; }
    void set_locals(LocalVariable* locals) { locals_ = locals; }
    int get_stack_size() const { return stack_size_; }
    void set_stack_size(int size) { stack_size_ = size; }
    
    Function* get_next() const { return next_; }
    void set_next(Function* fn) { next_ = fn; }
    std::string_view get_name() const { return name_; }
    void set_name(std::string_view n) { name_ = n; }

    const std::vector<LocalVariable*>& get_params() const { return params_; }
    void set_params(std::vector<LocalVariable*> params) { params_ = std::move(params); }

private:
    Node* body_ = nullptr;
    LocalVariable* locals_ = nullptr;
    int stack_size_ = 0;
    Function* next_=nullptr;
    std::string_view name_;

    std::vector<LocalVariable*> params_;
};

// Top level program object. It's an abstraction of translate unit in LLVM IR.
class Program {
    public:
    Program(Function* function_begin = nullptr, GlobalVariable* global_begin = nullptr, std::string_view file_name = {})
        : function_begin_(function_begin), file_name_(file_name), global_variable_begin_(global_begin) {}
    
        Function* get_function_begin() const { return function_begin_; }
        GlobalVariable* get_global_variable_begin() const { return global_variable_begin_; }

        void append_function(Function* fn) {
            if(!function_begin_) {
                function_begin_ = fn;
                return;
            } else {
                Function* cur = function_begin_;
                while (cur->get_next()) {
                    cur = cur->get_next();
                }
                cur->set_next(fn);
            }
        }

        void append_global_variable(GlobalVariable* gv) {
            if(!global_variable_begin_) {
                global_variable_begin_ = gv;
                return;
            } else {
                GlobalVariable* cur = global_variable_begin_;
                while (cur->get_next()) {
                    cur = cur->get_next();
                }
                cur->set_next(gv);
            }
        }
        
    private:
        // TODO: gloable valuables , typedef , .....
        Function* function_begin_ = nullptr;
        std::string_view file_name_;
        GlobalVariable* global_variable_begin_ = nullptr;

};

// AST node kind. Which of lhs/rhs/body/condition/then/els/init/inc/var/args is valid depends on kind.
enum class NodeKind {
    ND_ADD, ND_SUB, ND_MUL, ND_DIV, ND_NUM, ND_NEG,
    ND_EQ, ND_NE, ND_LT, ND_LE,
    ND_EXPR_STMT, ND_ASSIGN, ND_VAR, ND_GVAR ,ND_RETURN, ND_BLOCK,
    ND_IF, ND_FOR, ND_ADDR, ND_DEREF, ND_FUNCALL,
};
// ParsedParam describes one parameter in a function declarator.
// It does not own the token or the type object; both are managed by ASTContext.
struct ParsedParam {
    Token* name_tok{};
    Type* type{};
};

// DeclaratorResult carries everything parsed from a declarator:
// the completed type, the declared identifier, and any function parameters.
struct DeclaratorResult {
    Type* type{};
    Token* name_tok{};
    std::vector<ParsedParam> params;
};

class Node {
private:
    NodeKind kind;
    Node* lhs = nullptr;
    Node* rhs = nullptr;
    Node* next = nullptr;
    Token* tok = nullptr;   // For error reporting.
    int val = 0;            // ND_NUM only.
    LocalVariable* var = nullptr;    // ND_VAR only.
    GlobalVariable* gvar = nullptr;    // ND_GVAR only.
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
    Node(NodeKind kind, LocalVariable* var) : kind(kind), var(var) {}
    Node(NodeKind kind, GlobalVariable* gvar) : kind(kind), gvar(gvar) {}

    NodeKind get_nodekind() const { return kind; }
    Token* get_tok() const { return tok; }
    void set_tok(Token* t) { tok = t; }
    Type* get_ty() const { return ty; }
    void set_ty(Type* t) { ty = t; }
    int get_val() const { return val; }
    Node* get_lhs() const { return lhs; }
    Node* get_rhs() const { return rhs; }
    Node* get_nextstmt() const { return next; }
    LocalVariable* get_var() const { return var; }

    void set_nextstmt(Node* n) { next = n; }
    void set_var(LocalVariable* v) { var = v; }
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

    GlobalVariable* get_gvar() const { return gvar; }
    void set_gvar(GlobalVariable* v) { gvar = v; }
};