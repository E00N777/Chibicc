#include "parser.h"
#include "tokenize.h"

//
Node* Parser::expr()
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
    Node* node=primary();
    for(;;)
    {
        if(Tkequal(this->current, "*"))
        {
            current=this->current->get_next();
            node=new Node(NodeKind::ND_MUL,node,primary());
            continue;
        }
        if(Tkequal(this->current, "/"))
        {
            current=this->current->get_next();
             node=new Node(NodeKind::ND_DIV,node,primary());
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


