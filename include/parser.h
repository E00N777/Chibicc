#pragma once
#include "astnode.h"
#include "tokenize.h"
#include <string_view>

class ASTContext;

// Parsing layers: parse() -> function() -> compound_stmt()/stmt()/declaration() -> expr()/assign()/...
// Types come from declspec()/declarator(); Obj is created in declaration() and linked in locals.
// add_type() is called from compound_stmt() after each stmt (in type.cpp), not inside the parser.
class Parser {
private:
    Token* current_;
    LocalVariable* locals = nullptr;  // Head of current function's local variable list; reset at start of each function().
    ASTContext& ctx_;
    Program* program_ = nullptr;

    // --- AST node construction ---
    Node* new_binary(NodeKind kind, Node* lhs, Node* rhs, Token* tok);
    Node* make_binary(NodeKind kind, Node* lhs, Node* rhs, Token* op_tok);
    Node* make_func_call(NodeKind kind,Token* op_tok,std::string_view func_name);
    Node* make_gvar_node(GlobalVariable* gvar, Token* tok);

    // --- Expression parsing (precedence climbing: expr -> assign -> equality -> ... -> primary) ---
    Node* primary();       // ( expr ) | ident [ ( args ) ] | num
    Node* funcall();       // ident "(" [ assign ( "," assign )* ] ")"
    Node* mul();           // * /
    Node* expr();
    Node* postfix();       // [ ]
    Node* unary();         // + - * &
    Node* equality();      // == !=
    Node* relational();    // >= > <= <
    Node* add();           // + - (uses type for ptr arithmetic)
    Node* expr_stmt();     // expr ; or ;
    Node* stmt();          // return | while | for | if | block | expr_stmt
    Node* assign();        // = (right-associative)
    Node* compound_stmt(); // { stmt* }

    // --- Local symbols: find_var looks up in locals; new_lvar adds and chains ---
    LocalVariable* find_var(Token* tok);
    Node* new_var_node(LocalVariable* var, Token* tok);
    LocalVariable* new_lvar(const std::string& name,Type* ty);

    // --- Typed add/sub: require add_type on operands, then dispatch int vs pointer ---
    Node* new_num(int val, Token* tok);
    Node* new_add(Node* lhs, Node* rhs, Token* tok);
    Node* new_sub(Node* lhs, Node* rhs, Token* tok);

    // --- Declarations: declspec = "int", declarator parses pointers plus an optional
    // function parameter list and returns the full declarator shape. ---
    Type* declspec();
    DeclaratorResult declarator(Type* basety);
    Type* type_suffix(Type* ty, std::vector<ParsedParam>& params);
    std::vector<ParsedParam> parse_function_params();
    std::vector<LocalVariable*> create_param_locals(const std::vector<ParsedParam>& params);
    Node* declaration();

    // --- Top-level: one function definition (declspec + declarator + "{" compound_stmt "}") ---
    Function* function();
    Function* parse_function_definition(const DeclaratorResult& decl);
    GlobalVariable* find_gvar_variable(Token* tok);

    void parse_external_declaration(Program* program); //function declaration
    void parse_global_declaration(Program* program, Type* basety, DeclaratorResult first); // global variable declaration

    // --- String literal: emitted as an anonymous global initialized with raw bytes. ---
    std::string new_unique_name();
    GlobalVariable* new_anon_gvar(Type* ty);
    GlobalVariable* new_string_literal(Token* tok);

    int eval_const_expr(Node* node);
public:
    explicit Parser(Token* tk,ASTContext& ctx) : current_(tk) ,ctx_(ctx){}

    Program* parse();

    // --- Token stream: consume/expect/check/peek/advance (parser owns current_, tokens live in ctx) ---
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
};
