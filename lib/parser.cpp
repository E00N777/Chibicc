#include "parser.h"
#include "astnode.h"
#include "tokenize.h"
#include "diagnostic.h"
#include "context.h"
#include "type.h"
#include <vector>

// --- Program: list of function definitions. Types come from declspec/declarator; Obj from declaration(). ---
Program* Parser::parse() {

    program_ = ctx_.make_program(); 
    while (peek()->get_kind() != TokenKind::EOF_TK) {
        parse_external_declaration(program_);
    }
    return program_;
    
}
// --- Top-level helper function ---
void Parser::parse_external_declaration(Program* program) {
    Type* basety = declspec();
    DeclaratorResult first = declarator(basety);

    
    if (first.type->get_kind() == TypeKind::TY_FUNC && check("{")) {
        Function* fn = parse_function_definition(first);
        program->append_function(fn);
        return;
    }

    parse_global_declaration(program, basety, first);
}

void Parser::parse_global_declaration(Program* program, Type* basety, DeclaratorResult first) {
    DeclaratorResult decl = std::move(first);
    while(true){

        if(decl.type->get_kind() == TypeKind::TY_FUNC) {
            diagnostic::error_tok(decl.name_tok, "expected a global variable declaration");
        }
        GlobalVariable* gv = ctx_.make_global_variable(decl.name_tok->get_content(), decl.type);
        program->append_global_variable(gv);

        if(consume("=")) {
            if (decl.type->get_kind() != TypeKind::TY_INT)
                diagnostic::error_tok(peek(), "global initializer is only supported for int");

            Node* init = assign();
            gv->set_init_val(eval_const_expr(init));
        }
        if(consume(";")) {
            return;
        }
        expect(",");
        decl = declarator(basety);
        
    }
}

int Parser::eval_const_expr(Node* node) {
    if (!node)
        diagnostic::error_at(peek()->get_content(), "expected a constant expression");

    switch (node->get_nodekind()) {
    case NodeKind::ND_NUM:
        return node->get_val();

    case NodeKind::ND_NEG:
        return -eval_const_expr(node->get_lhs());

    case NodeKind::ND_ADD:
        return eval_const_expr(node->get_lhs()) + eval_const_expr(node->get_rhs());

    case NodeKind::ND_SUB:
        return eval_const_expr(node->get_lhs()) - eval_const_expr(node->get_rhs());

    case NodeKind::ND_MUL:
        return eval_const_expr(node->get_lhs()) * eval_const_expr(node->get_rhs());

    case NodeKind::ND_DIV: {
        long rhs = eval_const_expr(node->get_rhs());
        if (rhs == 0)
            diagnostic::error_tok(node->get_tok(), "division by zero in constant expression");
        return eval_const_expr(node->get_lhs()) / rhs;
    }

    case NodeKind::ND_EQ:
        return eval_const_expr(node->get_lhs()) == eval_const_expr(node->get_rhs());

    case NodeKind::ND_NE:
        return eval_const_expr(node->get_lhs()) != eval_const_expr(node->get_rhs());

    case NodeKind::ND_LT:
        return eval_const_expr(node->get_lhs()) < eval_const_expr(node->get_rhs());

    case NodeKind::ND_LE:
        return eval_const_expr(node->get_lhs()) <= eval_const_expr(node->get_rhs());

    default:
        diagnostic::error_tok(node->get_tok(), "not a constant expression");
    }
}


GlobalVariable* Parser::find_gvar_variable(Token* tok) {
    std::string_view name = tok->get_content();
    for (GlobalVariable* gv = program_->get_global_variable_begin(); gv; gv = gv->get_next())
        if (gv->get_name() == name)
            return gv;
    return nullptr;
}

Node* Parser::make_gvar_node(GlobalVariable* gvar, Token* tok) {
    Node* node = ctx_.make_node(NodeKind::ND_GVAR, gvar);
    node->set_tok(tok);
    return node;
}

// --- Local symbols: find by name in current function's locals list ---
LocalVariable* Parser::find_var(Token* tok) {
    std::string_view name = tok->get_content();
    for (LocalVariable* var = locals; var; var = var->get_next())
        if (var->get_name() == name)
            return var;
    return nullptr;
}

Node* Parser::new_var_node(LocalVariable* var, Token* tok) {
    Node* node = ctx_.make_node(NodeKind::ND_VAR, var);
    node->set_tok(tok);
    return node;
}

LocalVariable* Parser::new_lvar(const std::string& name, Type* ty) {
    LocalVariable* var = ctx_.make_obj(name, ty, locals);
    locals = var;
    return var;
}

// --- Binary / function-call node helpers ---
Node* Parser::new_binary(NodeKind kind, Node* lhs, Node* rhs, Token* tok) {
    Node* node = ctx_.make_node(kind, lhs, rhs);
    node->set_tok(tok);
    return node;
}

Node* Parser::make_binary(NodeKind kind, Node* lhs, Node* rhs, Token* op_tok) {
    return new_binary(kind, lhs, rhs, op_tok);
}

Node* Parser::make_func_call(NodeKind kind,Token* op_tok,std::string_view func_name)
{
    Node* node=ctx_.make_node(kind,nullptr,nullptr);
    node->set_tok(op_tok);
    node->set_func_name(func_name);
    return node;
}

// --- Literal and typed add/sub (parser calls add_type so +/- can distinguish int vs pointer) ---
Node* Parser::new_num(int val, Token* tok) {
    Node* node = ctx_.make_node(val);
    node->set_tok(tok);
    return node;
}

// int+int: plain add. ptr+int / int+ptr: scale int by 8 (pointer size), then add.
Node* Parser::new_add(Node* lhs, Node* rhs, Token* tok) {
    add_type(lhs, ctx_);
    add_type(rhs, ctx_);

    if (is_integer_or_char(lhs->get_ty(), ctx_) && is_integer_or_char(rhs->get_ty(), ctx_))
        return new_binary(NodeKind::ND_ADD, lhs, rhs, tok);

    if (lhs->get_ty()->get_base() && rhs->get_ty()->get_base())
        diagnostic::error_tok(tok, "invalid operands");

    if (!lhs->get_ty()->get_base() && rhs->get_ty()->get_base())
        std::swap(lhs, rhs);
    int scale = lhs->get_ty()->get_base()->get_size();
    rhs = new_binary(NodeKind::ND_MUL, rhs, new_num(scale, tok), tok);
    return new_binary(NodeKind::ND_ADD, lhs, rhs, tok);
}


// int-int: sub. ptr-int: scale then sub. ptr-ptr: byte difference / 8.
Node* Parser::new_sub(Node* lhs, Node* rhs, Token* tok) {
    add_type(lhs, ctx_);
    add_type(rhs, ctx_);

    if (is_integer_or_char(lhs->get_ty(), ctx_) && is_integer_or_char(rhs->get_ty(), ctx_))
        return new_binary(NodeKind::ND_SUB, lhs, rhs, tok);

    int scale = lhs->get_ty()->get_base()->get_size();

    if (lhs->get_ty()->get_base() && is_integer_or_char(rhs->get_ty(), ctx_)) {
        rhs = new_binary(NodeKind::ND_MUL, rhs, new_num(scale, tok), tok);
        add_type(rhs, ctx_);
        Node* node = new_binary(NodeKind::ND_SUB, lhs, rhs, tok);
        node->set_ty(lhs->get_ty());
        return node;
    }

    if (lhs->get_ty()->get_base() && rhs->get_ty()->get_base()) {
        Node* node = new_binary(NodeKind::ND_SUB, lhs, rhs, tok);
        node->set_ty(get_ty_int( ctx_));
        return new_binary(NodeKind::ND_DIV, node, new_num(scale, tok), tok);
    }

    diagnostic::error_tok(tok, "invalid operands");
}

// --- Declaration specifier: currently only "int" ---
Type* Parser::declspec()
{
    if(consume("int")) {
        return ctx_.get_int_type();
    }
    if(consume("char")) {
        return ctx_.get_char_type();
    }
    diagnostic::error_at(peek()->get_content(), "expected int or char");
}

Type* Parser::type_suffix(Type* ty, std::vector<ParsedParam>& params)
{
    if (consume("(")) {
        params = parse_function_params();
        expect(")");

        std::vector<Type*> param_types;
        param_types.reserve(params.size());
        for (const ParsedParam& param : params)
            param_types.push_back(param.type);

        return ctx_.make_func_type(ty, std::move(param_types)); 
    }

    if (consume("[")) {
        int len  = peek()->get_number();
        advance();
        expect("]");
        ty = type_suffix(ty, params);
        return ctx_.make_array_type(ty, len);
    }
    return ty;
}

// Parse a function parameter list after the opening '('.
// Each parameter is parsed using the same declarator logic as local variables so
// pointer parameters naturally work once pointer declarators are supported.
std::vector<ParsedParam> Parser::parse_function_params() {
    std::vector<ParsedParam> params;

    if (check(")"))
        return params;

    while (true) {
        Type* param_basety = declspec();
        DeclaratorResult param_decl = declarator(param_basety);
        params.push_back({param_decl.name_tok, param_decl.type});

        if (!consume(","))
            return params;
    }
}

// Parameters must be visible as local variables inside the function body.
// Because new_lvar prepends to the locals list, we create them in reverse order
// so the final linked list preserves the original source order.
std::vector<LocalVariable*> Parser::create_param_locals(const std::vector<ParsedParam>& params) {
    std::vector<LocalVariable*> param_objs(params.size(), nullptr);

    for (std::size_t i = params.size(); i > 0; --i) {
        const ParsedParam& param = params[i - 1];
        LocalVariable* var = new_lvar(std::string(param.name_tok->get_content()), param.type);
        param_objs[i - 1] = var;
    }

    return param_objs;
}

// Declarator: * * ident [ "(" func-params? ")" ].
// The returned structure includes the fully built type, the declared name token,
// and the parsed parameter metadata when the declarator is a function.
DeclaratorResult Parser::declarator(Type* basety)
{
    Type* ty = basety;
    while (consume("*"))
        ty = ctx_.make_ptr_type(ty);

    if (peek()->get_kind() != TokenKind::IDENT)
        diagnostic::error_at(peek()->get_content(), "expected an identifier");
    Token* ident_tok=peek();
    advance();

    std::vector<ParsedParam> params;
    ty = type_suffix(ty, params);

    return {ty, ident_tok, std::move(params)};
}

// int a, b = 3; : declspec once, then per-declarator type + new_lvar; optional init as assign stmt.
Node* Parser::declaration()
{
    Token* decl_tok = peek();
    Type* basety = declspec();

    Node head(NodeKind::ND_EXPR_STMT);
    Node* cur = &head;
    bool first = true;
    while (!check(";")) {
        if (!first) 
        {
            expect(",");
        }
        first=false;
        
        DeclaratorResult decl = declarator(basety);
        Type* ty = decl.type;
        Token* ident_tok = decl.name_tok;
        std::string name(ident_tok->get_content());
        LocalVariable* var=new_lvar(name,ty);

        if(!check("="))
            continue;

        Token* assign_tk=peek();
        consume("=");
        Node* rhs=assign();
        Node* lhs= new_var_node(var,ident_tok);
        Node* assign_node=make_binary(NodeKind::ND_ASSIGN,lhs,rhs,assign_tk);
        Node* stmt_node=ctx_.make_node(NodeKind::ND_EXPR_STMT,assign_node);
        stmt_node->set_tok(assign_tk);
        cur->set_nextstmt(stmt_node);
        cur=stmt_node;
    }
    expect(";");

    Node* node=ctx_.make_node(NodeKind::ND_BLOCK);
    node->set_tok(decl_tok);
    node->set_body(head.get_nextstmt());
    return node;
}

// --- Compound statement: { stmt* }. Each stmt is declaration() or stmt(). add_type called after each. ---
Node* Parser::compound_stmt()
{
    Token* block_tok = peek();
    Node head(NodeKind::ND_EXPR_STMT);
    Node* cur = &head;

    while (!check("}")) {
        if (check("int") || check("char"))
            cur->set_nextstmt(declaration());
        else
            cur->set_nextstmt(stmt());
        cur = cur->get_nextstmt();
        add_type(cur,ctx_);
    }
    Node* node = ctx_.make_node(NodeKind::ND_BLOCK);
    node->set_tok(block_tok);
    node->set_body(head.get_nextstmt());
    expect("}");
    return node;
}

// --- Statement: return | while | for | if | block | expr ; ---
Node* Parser::stmt()
{
    if (check("return")) {
        Token* ret_tok = peek();
        consume("return");
        Node* node = ctx_.make_node(NodeKind::ND_RETURN, expr());
        node->set_tok(ret_tok);
        expect(";");
        return node;
    }

    if (check("while")) {
        Token* while_tok = peek();
        consume("while");
        Node* node = ctx_.make_node(NodeKind::ND_FOR);
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
        Node* node = ctx_.make_node(NodeKind::ND_FOR);
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
        Node* node = ctx_.make_node(NodeKind::ND_IF);
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
        Node* node = ctx_.make_node(NodeKind::ND_BLOCK);
        node->set_tok(stmt_tok);
        return node;
    }
    Node* node = ctx_.make_node(NodeKind::ND_EXPR_STMT, expr());
    node->set_tok(stmt_tok);
    expect(";");
    return node;
}

// --- Expression: assign is top level (right-associative =), then equality, relational, add, mul, unary, primary ---
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

// Parse >=, >, <=, < (relational operators).
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

// Primary: ( expr ) | ident [ ( args ) ] | num. Ident + "(" -> funcall; else find_var and make ND_VAR.
Node* Parser::primary()
{
    if (consume("(")) {
        Node* node = expr();
        expect(")");
        return node;
    }
    if (peek()->get_kind() == TokenKind::IDENT) {
        if (is_followed_by("("))
            return funcall();
        Token* ident_tok = peek();

        if(LocalVariable* var = find_var(peek())) {
            if (!var)
                diagnostic::error_tok(ident_tok, "undefined variable");
            advance();
            return new_var_node(var, ident_tok);
        }

        if(GlobalVariable* gvar = find_gvar_variable(peek())) {
            if (!gvar)
                diagnostic::error_tok(ident_tok, "undefined variable");
            advance();
            return make_gvar_node(gvar, ident_tok);
        }
        diagnostic::error_tok(ident_tok, "undefined variable");    
    }

    if (peek()->get_kind() == TokenKind::NUM) {
        Token* num_tok = peek();
        Node* node = ctx_.make_node(peek()->get_number());
        node->set_tok(num_tok);
        advance();
        return node;
    }
    diagnostic::error_at(peek()->get_content(), "expected an expression");
}

// Postfix: operator [ ]  
// a[b]  ==  *(a + b)
Node* Parser::postfix() {

    Node* node = primary();

    while(check("[")) {
        Token* bracket_tok = peek();
        consume("[");
        Node* index = expr();
        expect("]");
        node = ctx_.make_node(NodeKind::ND_DEREF,new_add(node,index,bracket_tok));
        node->set_tok(bracket_tok);
    }

    return node;
}

// Unary: + - * & (then unary) or postfix.
// sizeof(expr) complier time calculation of the size of the expression
Node* Parser::unary()
{
    if (consume("+"))
        return unary();
    if (check("-")) {
        Token* minus_tok = peek();
        consume("-");
        Node* node = ctx_.make_node(NodeKind::ND_NEG, unary());
        node->set_tok(minus_tok);
        return node;
    }
    if (check("&")) {
        Token* addr_tok = peek();
        consume("&");
        Node* node = ctx_.make_node(NodeKind::ND_ADDR, unary());
        node->set_tok(addr_tok);
        return node;
    }
    if (check("*")) {
        Token* deref_tok = peek();
        consume("*");
        Node* node = ctx_.make_node(NodeKind::ND_DEREF, unary());
        node->set_tok(deref_tok);
        return node;
    }
    if(check("sizeof")) {
        Token* sizeof_tok = peek();
        consume("sizeof");
        Node* node = unary();
        add_type(node, ctx_);
        return new_num(node->get_ty()->get_size(), sizeof_tok);
    }
    return postfix();
}

// Function call: ident ( [ assign ( "," assign )* ] ). Args stored as list via set_nextstmt.
Node* Parser::funcall()
{
    Token* func_name_tok = peek();
    advance();
    expect("(");
    Node head(NodeKind::ND_EXPR_STMT);
    Node* cur=&head;
    while(!check(")"))
    {
        if(consume("void"))
            continue;
        if(cur!=&head)
            expect(",");
        Node* arg=assign();
        cur->set_nextstmt(arg);
        cur=arg;
    }
    expect(")");
    Node* node = make_func_call(NodeKind::ND_FUNCALL, func_name_tok, func_name_tok->get_content());
    node->set_args(head.get_nextstmt());
    return node;
}

Function* Parser::parse_function_definition(const DeclaratorResult& decl) {

    locals = nullptr;

    // Function parameters behave like predeclared local variables whose initial
    // values come from the calling convention rather than from an assignment in
    // the source program.
    std::vector<LocalVariable*> params = create_param_locals(decl.params);
    expect("{");
    Node* body = compound_stmt();
    Function* fn = ctx_.make_function(body, locals, decl.name_tok->get_content());
    fn->set_params(std::move(params));
    return fn;
}

// One function: declspec + declarator (name + optional () for function type), then { compound_stmt }.
// locals reset so this function's declarations build a fresh list; that list is stored in the Function.
Function* Parser::function()
{
    // Reset the local symbol list before starting a new function so parameters
    // and local declarations for different functions never mix together.
    Type* basety = declspec();
    DeclaratorResult decl = declarator(basety);
    
    if(decl.type->get_kind()!=TypeKind::TY_FUNC) {
        diagnostic::error_tok(decl.name_tok, "expected a function declaration");
    }

    return parse_function_definition(decl);
    
}
