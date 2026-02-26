#include "parser.h"
#include "astnode.h"
#include "tokenize.h"
#include "diagnostic.h"


// Find a local variable by name.
Obj* Parser::find_var(Token* tok) {
    std::string_view name = tok->get_content();
    for (Obj* var = locals; var; var = var->get_next())
        if (var->get_name() == name)
            return var;
    return nullptr;
}

Node* Parser::new_var_node(Obj* var, Token* tok) {
    Node* node = new Node(NodeKind::ND_VAR, var);
    node->set_tok(tok);
    return node;
}

Obj* Parser::new_lvar(const std::string& name) {
    Obj* var = new Obj(name, locals);
    locals = var;
    return var;
}
Node* Parser::compound_stmt()
{
    Token* block_tok = current;  // first token inside block
    Node head(NodeKind::ND_EXPR_STMT);  // sentinel for statement list
    Node* cur = &head;
    while(!Tkequal(current,"}"))
    {
        Node* stmt_node = stmt();
        cur->set_nextstmt(stmt_node);
        cur = cur->get_nextstmt();
    }
    Node* node = new Node(NodeKind::ND_BLOCK);
    node->set_tok(block_tok);
    node->set_body(head.get_nextstmt());
    Tkskip(current,"}");
    return node;
}

Function* Parser::parse() {
    Tkskip(current,"{");
    Function* prog = new Function();
    prog->set_body(compound_stmt());
    prog->set_locals(locals);
    return prog;
}

Node* Parser::stmt()
{
    if(Tkequal(current,"return"))
    {
        Token* ret_tok = current;
        current=this->current->get_next();
        Node* node=new Node(NodeKind::ND_RETURN,expr());
        node->set_tok(ret_tok);
        Tkskip(current,";");
        return node;
    }

    if(Tkequal(current,"while"))
    {
        Token* while_tok = current;
        Node* node = new Node(NodeKind::ND_FOR);
        node->set_tok(while_tok);
        current=this->current->get_next();
        Tkskip(current,"(");
        node->set_condition(expr());
        Tkskip(current,")");
        node->set_then(stmt());
        return node;
    }

    if(Tkequal(current,"for"))
    {
        Token* for_tok = current;
        Node* node = new Node(NodeKind::ND_FOR);
        node->set_tok(for_tok);
        current=this->current->get_next();
        Tkskip(current,"(");
        node->set_init(expr_stmt());
        if(!Tkequal(current,";"))
        {
            node->set_condition(expr());
        }
        Tkskip(current,";");
        if(!Tkequal(current,")"))
        {
            node->set_inc(expr());
        }
        Tkskip(current,")");
        node->set_then(stmt());
        return node;
    }
    
    if(Tkequal(current,"if"))
    {
        Token* if_tok = current;
        Node* node = new Node(NodeKind::ND_IF);
        node->set_tok(if_tok);
        current=this->current->get_next();
        Tkskip(current,"(");
        node->set_condition(expr());
        Tkskip(current,")");
        node->set_then(stmt());
        if(Tkequal(current,"else"))
        {
            current=this->current->get_next();
            node->set_els(stmt());
        }
        return node;
    }

    if(Tkequal(current,"{"))
    {
        Tkskip(current,"{");
        Node* node = compound_stmt();
        return node;
    }

    return expr_stmt();
}

Node* Parser::expr_stmt()
{
    Token* stmt_tok = current;
    if(Tkequal(current,";"))
    {
        Tkskip(current,";");
        Node* node=new Node(NodeKind::ND_BLOCK);
        node->set_tok(stmt_tok);
        return node;
    }
    Node* node =new Node(NodeKind::ND_EXPR_STMT,expr());
    node->set_tok(stmt_tok);
    Tkskip(current,";");
    return node;
}


Node* Parser::assign()
{
    Node* node = equality();

    if(Tkequal(current,"="))
    {
        Token* assign_tok = current;
        current = this->current->get_next();
        Node* rhs = assign();
        node = new Node(NodeKind::ND_ASSIGN, node, rhs);
        node->set_tok(assign_tok);
    }
    return node;
}

//lowest level
Node* Parser::expr()
{
    return assign();
}

Node* Parser::equality()
{
    Node* node=relational();

    for(;;)
    {
        if(Tkequal(this->current,"==" ))
        {
            Token* op_tok = current;
            current=this->current->get_next();
            node=new Node(NodeKind::ND_EQ,node,relational());
            node->set_tok(op_tok);
            continue;
        }

        if(Tkequal(this->current,"!=" ))
        {
            Token* op_tok = current;
            current=this->current->get_next();
            node=new Node(NodeKind::ND_NE,node,relational());
            node->set_tok(op_tok);
            continue;
        }

        return node;
    }
    
}

Node* Parser::relational()
{
    Node* node=add();
    for(;;)
    {
        if(Tkequal(this->current,">=" ))
        {
            Token* op_tok = current;
            current=this->current->get_next();
            node=new Node(NodeKind::ND_LE,add(),node);
            node->set_tok(op_tok);
            continue;
        }
        if(Tkequal(this->current,">" ))
        {
            Token* op_tok = current;
            current=this->current->get_next();
            node=new Node(NodeKind::ND_LT,add(),node);
            node->set_tok(op_tok);
            continue;
        }
        if(Tkequal(this->current,"<=" ))
        {
            Token* op_tok = current;
            current=this->current->get_next();
            node=new Node(NodeKind::ND_LE,node,add());
            node->set_tok(op_tok);
            continue;
        }
        if(Tkequal(this->current,"<" ))
        {
            Token* op_tok = current;
            current=this->current->get_next();
            node=new Node(NodeKind::ND_LT,node,add());
            node->set_tok(op_tok);
            continue;
        }

        return node;
    }
    
}


Node* Parser::add()
{
    Node* node=mul();

    for(;;)
    {
        if(Tkequal(this->current, "+"))
        {
            Token* op_tok = current;
            current=this->current->get_next();
            node=new Node(NodeKind::ND_ADD,node,mul());
            node->set_tok(op_tok);
            continue;
        }
        if(Tkequal(this->current, "-"))
        {
            Token* op_tok = current;
            current=this->current->get_next();
            node=new Node(NodeKind::ND_SUB,node,mul());
            node->set_tok(op_tok);
            continue;
        }
        return node;
    }
}

// Mid level
Node* Parser::mul()
{
    Node* node=unary();
    for(;;)
    {
        if(Tkequal(this->current, "*"))
        {
            Token* op_tok = current;
            current=this->current->get_next();
            node=new Node(NodeKind::ND_MUL,node,unary());
            node->set_tok(op_tok);
            continue;
        }
        if(Tkequal(this->current, "/"))
        {
            Token* op_tok = current;
            current=this->current->get_next();
            node=new Node(NodeKind::ND_DIV,node,unary());
            node->set_tok(op_tok);
            continue;
        }
        return node;
    }
}

// Top level
Node* Parser::primary()
{
    if(Tkequal(this->current,"("))
    {
        current=this->current->get_next();//skip operator (
        Node* node=expr();
        Tkskip(current,")");
        return node;
    }
    if (current->get_kind() == TokenKind::IDENT) {
        Token* ident_tok = current;
        Obj* var = find_var(current);
        if (!var)
            var = new_lvar(std::string(current->get_content()));
        current = current->get_next();
        return new_var_node(var, ident_tok);
    }
    if(this->current->get_kind()==TokenKind::NUM)
    {
        Token* num_tok = current;
        Node* node=new Node(current->get_number());
        node->set_tok(num_tok);
        current=current->get_next();
        return node;
    }
    diagnostic::error_at(current->get_content(), "expected an expression");
}   

// unary = ("+" | "-" | "*" | "&") unary
Node* Parser::unary()
{
    if(Tkequal(this->current,"+"))
    {
        current=this->current->get_next();
        return unary();
    }
    if(Tkequal(this->current,"-"))
    {
        Token* minus_tok = current;
        current=this->current->get_next();
        Node* node = new Node(NodeKind::ND_NEG,unary());
        node->set_tok(minus_tok);
        return node;
    }
    if(Tkequal(this->current,"&"))
    {
        
        Token* addr_tok = current;
        current=this->current->get_next();
        Node* node=new Node(NodeKind::ND_ADDR,unary());
        node->set_tok(addr_tok);
        return node;
    }
    if(Tkequal(this->current,"*"))
    {
        
        Token* deref_tok = current;
        current=this->current->get_next();
        Node* node=new Node(NodeKind::ND_DEREF,unary());
        node->set_tok(deref_tok);
        return node;
    }

    return primary();
}