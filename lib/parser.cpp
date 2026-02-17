#include "parser.h"
#include "astnode.h"
#include "tokenize.h"

Node* Parser::program()
{
    Node head(NodeKind::ND_EXPR_STMT);
    Node* cur= &head;
    while (this->current->get_kind() != TokenKind::EOF_TK)
    {
        Node* stmt_node=stmt();
        cur->set_nextstmt(stmt_node);
        cur=cur->get_nextstmt();
    }
    return head.get_nextstmt();
}

Node* Parser::stmt()
{
    return expr_stmt();
}

Node* Parser::expr_stmt()
{
    Node* node =new Node(NodeKind::ND_EXPR_STMT,expr());
    current =Tkskip(current,";");
    return node;
}


//lowest level
Node* Parser::expr()
{
    return equality();
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
    errorat(current->get_content(),"Parsering failed");
}

//Mid level
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
    errorat(current->get_content(),"Parsering failed");
}

//Top level
Node* Parser::primary()
{
    if(Tkequal(this->current,"("))
    {
        current=this->current->get_next();//skip operator (
        Node* node=expr();
        current=Tkskip(this->current,")");
        return node;
    }
    if(this->current->get_kind()==TokenKind::NUM)
    {
        Node* node=new Node(current->get_number());
        current=current->get_next();
        return node;
    }
    errorat(current->get_content(),"expected a experssion");
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