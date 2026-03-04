#include "parser.h"
#include "astnode.h"
#include "tokenize.h"
#include "diagnostic.h"
#include "type.h"
#include <utility>

// Entry point: parse the whole program (expects "{" then compound_stmt); returns the function with body and locals.
Function* Parser::parse() {
    expect("{");
    Function* prog = new Function();
    prog->set_body(compound_stmt());
    prog->set_locals(locals);
    return prog;
}

//=================================== Variable and local symbol management ===================================
// Find a local variable by name in the current scope (locals list).
Obj* Parser::find_var(Token* tok) {
    std::string_view name = tok->get_content();
    for (Obj* var = locals; var; var = var->get_next())
        if (var->get_name() == name)
            return var;
    return nullptr;
}

// Create an AST node for a variable reference.
Node* Parser::new_var_node(Obj* var, Token* tok) {
    Node* node = new Node(NodeKind::ND_VAR, var);
    node->set_tok(tok);
    return node;
}

// Allocate a new local variable and prepend it to the locals list.
Obj* Parser::new_lvar(const std::string& name,Type* ty) {
    Obj* var = new Obj(name, ty, locals);
    locals = var;
    return var;
}
//=================================== End of Variable and local symbol management ===================================

//=================================== Generic AST node construction ===================================
// Create a binary-operation AST node (e.g. add, sub, assign).
Node* Parser::new_binary(NodeKind kind, Node* lhs, Node* rhs, Token* tok) {
    Node* node = new Node(kind, lhs, rhs);
    node->set_tok(tok);
    return node;
}

// Wrapper for new_binary; builds a binary node with the given operator token.
Node* Parser::make_binary(NodeKind kind, Node* lhs, Node* rhs, Token* op_tok) {
    return new_binary(kind, lhs, rhs, op_tok);
}

Node* Parser::make_func_call(NodeKind kind,Token* op_tok,std::string_view func_name)
{
    Node* node=new Node(kind,nullptr,nullptr);
    node->set_tok(op_tok);
    node->set_func_name(func_name);
    return node;
}

//=================================== End of Generic AST node construction ===================================

//=================================== Pointer arithmetic ===================================
// Create an integer literal AST node.
Node* Parser::new_num(int val, Token* tok) {
    Node* node = new Node(val);
    node->set_tok(tok);
    return node;
}

// int+int => add; ptr+int or int+ptr => scale int by 8 then add.
Node* Parser::new_add(Node* lhs, Node* rhs, Token* tok) {
    add_type(lhs);
    add_type(rhs);

    if (is_integer(lhs->get_ty()) && is_integer(rhs->get_ty()))
        return new_binary(NodeKind::ND_ADD, lhs, rhs, tok);

    if (lhs->get_ty()->get_base() && rhs->get_ty()->get_base())
        diagnostic::error_tok(tok, "invalid operands");

    // Canonicalize num+ptr => ptr+num.
    if (!lhs->get_ty()->get_base() && rhs->get_ty()->get_base())
        std::swap(lhs, rhs);

    // ptr + num: scale rhs by 8 (element size).
    rhs = new_binary(NodeKind::ND_MUL, rhs, new_num(8, tok), tok);
    return new_binary(NodeKind::ND_ADD, lhs, rhs, tok);
}


// int-int => sub; ptr-int => scale then sub; ptr-ptr => byte diff / 8.
Node* Parser::new_sub(Node* lhs, Node* rhs, Token* tok) {
    add_type(lhs);
    add_type(rhs);

    if (is_integer(lhs->get_ty()) && is_integer(rhs->get_ty()))
        return new_binary(NodeKind::ND_SUB, lhs, rhs, tok);

    if (lhs->get_ty()->get_base() && is_integer(rhs->get_ty())) {
        rhs = new_binary(NodeKind::ND_MUL, rhs, new_num(8, tok), tok);
        add_type(rhs);
        Node* node = new_binary(NodeKind::ND_SUB, lhs, rhs, tok);
        node->set_ty(lhs->get_ty());
        return node;
    }

    if (lhs->get_ty()->get_base() && rhs->get_ty()->get_base()) {
        Node* node = new_binary(NodeKind::ND_SUB, lhs, rhs, tok);
        node->set_ty(get_ty_int());
        return new_binary(NodeKind::ND_DIV, node, new_num(8, tok), tok);
    }

    diagnostic::error_tok(tok, "invalid operands");
}
//=================================== End of Pointer arithmetic ===================================

//=================================== Declaration parsing ===================================
// Parse a declaration specifier: int
Type* Parser::declspec()
{
    expect("int");
    return get_ty_int();
}

std::pair<Type*,Token*> Parser::declarator(Type* basety)
{
    Type* ty=basety;
    while(consume("*")) // Parse pointer types (e.g. int*,int** ...etc)
        ty=Type::pointer_to(ty);

    if(peek()->get_kind()!=TokenKind::IDENT)
        diagnostic::error_at(peek()->get_content(), "expected an identifier");
    Token* ident_tok=peek();
    advance();
    return {ty,ident_tok};
}


Node* Parser::declaration()
{
    Token* decl_tok=peek();
    Type *basety=declspec(); //consume "int"

    Node head(NodeKind::ND_EXPR_STMT);
    Node* cur=&head;
    bool first=true; // first declaration in the list
    //int a,b=10;
    while(!check(";"))
    {
        // for multiple declarations
        if(!first) 
        {
            expect(",");
        }
        first=false;
        
        auto [ty,ident_tok]=declarator(basety);
        std::string name(ident_tok->get_content());
        Obj* var=new_lvar(name,ty);

        if(!check("="))
            continue;

        Token* assign_tk=peek();
        consume("=");
        Node* rhs=assign();
        Node* lhs= new_var_node(var,ident_tok);
        Node* assign_node=make_binary(NodeKind::ND_ASSIGN,lhs,rhs,assign_tk);
        Node* stmt_node=new Node(NodeKind::ND_EXPR_STMT,assign_node);
        stmt_node->set_tok(assign_tk);
        cur->set_nextstmt(stmt_node);
        cur=stmt_node;
    }
    expect(";");

    Node* node=new Node(NodeKind::ND_BLOCK);
    node->set_tok(decl_tok);
    node->set_body(head.get_nextstmt());
    return node;

}
//=================================== End of Declaration parsing ===================================


//=================================== Top-level parse and compound/statement ===================================
// Parse a compound statement: { stmt; stmt; ... } and return a block node.
Node* Parser::compound_stmt()
{
    Token* block_tok = peek();
    Node head(NodeKind::ND_EXPR_STMT);  // sentinel for statement list
    Node* cur = &head;

    while (!check("}")){
        if(check("int")){
            cur->set_nextstmt(declaration());
        }else {
            cur->set_nextstmt(stmt());
        }
        cur = cur->get_nextstmt();
        add_type(cur);
    }
    Node* node = new Node(NodeKind::ND_BLOCK);
    node->set_tok(block_tok);
    node->set_body(head.get_nextstmt());
    expect("}");
    return node;
}


//=================================== End of Top-level parse and compound/statement ===================================

//=================================== Statement parsing ===================================
// Parse a single statement: return, while, for, if, block, or expression statement.
Node* Parser::stmt()
{
    if (check("return")) {
        Token* ret_tok = peek();
        consume("return");
        Node* node = new Node(NodeKind::ND_RETURN, expr());
        node->set_tok(ret_tok);
        expect(";");
        return node;
    }

    if (check("while")) {
        Token* while_tok = peek();
        consume("while");
        Node* node = new Node(NodeKind::ND_FOR);
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
        Node* node = new Node(NodeKind::ND_FOR);
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
        Node* node = new Node(NodeKind::ND_IF);
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

// Parse an expression statement (expr;) or empty statement (;).
Node* Parser::expr_stmt()
{
    Token* stmt_tok = peek();
    if (consume(";")) {
        Node* node = new Node(NodeKind::ND_BLOCK);
        node->set_tok(stmt_tok);
        return node;
    }
    Node* node = new Node(NodeKind::ND_EXPR_STMT, expr());
    node->set_tok(stmt_tok);
    expect(";");
    return node;
}
//=================================== End of Statement parsing ===================================

//=================================== Expression parsing — assignment and top level ===================================
// Parse assignment (right-associative); also parses equality and below.
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

// Top-level expression entry; currently just delegates to assign (lowest precedence).
Node* Parser::expr()
{
    return assign();
}
//=================================== End of Expression parsing — assignment and top level ===================================

//=================================== Expression parsing — equality and relational ===================================
// Parse == and != (equality operators).
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
//=================================== End of Expression parsing — equality and relational ===================================

//=================================== Expression parsing — additive ===================================
// Parse + and - (additive operators); uses typed new_add/new_sub for pointer arithmetic.
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
//=================================== End of Expression parsing — additive ===================================

//=================================== Expression parsing — multiplicative ===================================
// Parse * and / (multiplicative operators).
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
//=================================== End of Expression parsing — multiplicative ===================================

//=================================== Expression parsing — unary and primary ===================================
// Parse primary expression: (expr), identifier, or number.
Node* Parser::primary()
{
    if (consume("(")) {
        Node* node = expr();
        expect(")");
        return node;
    }
    if (peek()->get_kind() == TokenKind::IDENT) {
        // Function call: ident followed by "(" (check next token, not current)
        if (is_followed_by("("))
        {
            return funcall();
        }
        //Variable name processing
        Token* ident_tok = peek();
        Obj* var = find_var(peek());
        if (!var)
            diagnostic::error_tok(ident_tok, "undefined variable");
        advance();
        return new_var_node(var, ident_tok);
    }
    if (peek()->get_kind() == TokenKind::NUM) {
        Token* num_tok = peek();
        Node* node = new Node(peek()->get_number());
        node->set_tok(num_tok);
        advance();
        return node;
    }
    diagnostic::error_at(peek()->get_content(), "expected an expression");
}

// Parse unary: +, -, *, & applied to unary, or primary.
Node* Parser::unary()
{
    if (consume("+"))
        return unary();
    if (check("-")) {
        Token* minus_tok = peek();
        consume("-");
        Node* node = new Node(NodeKind::ND_NEG, unary());
        node->set_tok(minus_tok);
        return node;
    }
    if (check("&")) {
        Token* addr_tok = peek();
        consume("&");
        Node* node = new Node(NodeKind::ND_ADDR, unary());
        node->set_tok(addr_tok);
        return node;
    }
    if (check("*")) {
        Token* deref_tok = peek();
        consume("*");
        Node* node = new Node(NodeKind::ND_DEREF, unary());
        node->set_tok(deref_tok);
        return node;
    }
    return primary();
}
//=================================== End of Expression parsing — unary and primary ===================================

//=================================== Expression parsing — function call ===================================
Node* Parser::funcall()
{
    Token* func_name_tok=peek();
    advance();
    expect("(");
    // Zero-arg call: accept () or (void)
    // if (consume(")"))
    //     { /* no args */ }
    // else if (consume("void"))
    //     expect(")");
    // else
    //     diagnostic::error_at(peek()->get_content(), "expected ) or void");

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
    
    Node* node=make_func_call(NodeKind::ND_FUNCALL,func_name_tok,func_name_tok->get_content());
    node->set_args(head.get_nextstmt());
    return node;
}