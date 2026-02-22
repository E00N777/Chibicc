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

Node* Parser::new_var_node(Obj* var) {
    Node* node = new Node(NodeKind::ND_VAR, var);
    return node;
}

Obj* Parser::new_lvar(const std::string& name) {
    Obj* var = new Obj(name, locals);
    locals = var;
    return var;
}
Node* Parser::compound_stmt()
{
    Node head(NodeKind::ND_EXPR_STMT);  // sentinel for statement list
    Node* cur = &head;
    while(!Tkequal(current,"}"))
    {
        Node* stmt_node = stmt();
        cur->set_nextstmt(stmt_node);
        cur = cur->get_nextstmt();
    }
    Node* node = new Node(NodeKind::ND_BLOCK);
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
        current=this->current->get_next();
        Node* node=new Node(NodeKind::ND_RETURN,expr());
        Tkskip(current,";");
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
    if(Tkequal(current,";"))
    {
        Tkskip(current,";");
        Node* node=new Node(NodeKind::ND_BLOCK);
        return node;
    }
    Node* node =new Node(NodeKind::ND_EXPR_STMT,expr());
    Tkskip(current,";");
    return node;
}


Node* Parser::assign()
{
    Node* node = equality();

    if(Tkequal(current,"="))
    {
        current = this->current->get_next();
        Node* rhs = assign();
        node = new Node(NodeKind::ND_ASSIGN, node, rhs);
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
            current=this->current->get_next();
            node=new Node(NodeKind::ND_EQ,node,relational());
            continue;
        }

        if(Tkequal(this->current,"!=" ))
        {
            current=this->current->get_next();
            node=new Node(NodeKind::ND_NE,node,relational());
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
            current=this->current->get_next();
            node=new Node(NodeKind::ND_LE,add(),node);
            continue;
        }
        if(Tkequal(this->current,">" ))
        {
            current=this->current->get_next();
            node=new Node(NodeKind::ND_LT,add(),node);
            continue;
        }
        if(Tkequal(this->current,"<=" ))
        {
            current=this->current->get_next();
            node=new Node(NodeKind::ND_LE,node,add());
            continue;
        }
        if(Tkequal(this->current,"<" ))
        {
            current=this->current->get_next();
            node=new Node(NodeKind::ND_LT,node,add());
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
            current=this->current->get_next();
            node=new Node(NodeKind::ND_ADD,node,mul());
            continue;
        }
        if(Tkequal(this->current, "-"))
        {   
            current=this->current->get_next();
            node=new Node(NodeKind::ND_SUB,node,mul());
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
            current=this->current->get_next();
            node=new Node(NodeKind::ND_MUL,node,unary());
            continue;
        }
        if(Tkequal(this->current, "/"))
        {
            current=this->current->get_next();
             node=new Node(NodeKind::ND_DIV,node,unary());
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
        Obj* var = find_var(current);
        if (!var)
            var = new_lvar(std::string(current->get_content()));
        current = current->get_next();
        return new_var_node(var);
    }
    if(this->current->get_kind()==TokenKind::NUM)
    {
        Node* node=new Node(current->get_number());
        current=current->get_next();
        return node;
    }
    diagnostic::error_at(current->get_content(), "expected an expression");
}   

Node* Parser::unary()
{
    if(Tkequal(this->current,"+"))
    {
        current=this->current->get_next();
        return unary();
    }
    if(Tkequal(this->current,"-"))
    {
        current=this->current->get_next();
        Node* node = new Node(NodeKind::ND_NEG,unary());
        return node;
    }

    return primary();
}