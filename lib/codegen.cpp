#include "codegen.h"
#include "astnode.h"
#include "diagnostic.h"
#include <iostream>


void CodeGen::push() {
    std::cout<<"    push %rax\n";
    depth++;
}
void CodeGen::pop(const char* reg){
    std::cout<<"    pop "<<reg<<"\n";
    depth--;
}

void CodeGen::gen_expr(Node* node)
{
    switch (node->get_nodekind()) {
    case NodeKind::ND_NUM:
        std::cout <<"    mov $" << node->get_val() << ", %rax\n";
        return;
    case NodeKind::ND_NEG:
        gen_expr(node->get_lhs());
        std::cout << "    neg %rax\n";
        return;
    }

    gen_expr(node->get_rhs());
    push();
    gen_expr(node->get_lhs());
    pop("%rdi");
    
    switch (node->get_nodekind()) {
    case NodeKind::ND_ADD:
        std::cout << "    add %rdi, %rax\n";
        break;
    case NodeKind::ND_SUB:
        std::cout << "    sub %rdi, %rax\n";
        break;
    case NodeKind::ND_MUL:
        std::cout << "    imul %rdi, %rax\n";
        break;
    case NodeKind::ND_DIV:
        std::cout << "    cqo\n";
        std::cout << "    idiv %rdi\n";
        break;
    case NodeKind::ND_LT:
    case NodeKind::ND_EQ:
    case NodeKind::ND_NE:
    case NodeKind::ND_LE:
        std::cout << "    cmp %rdi, %rax\n";
        if (node->get_nodekind() == NodeKind::ND_EQ)
            std::cout << "    sete %al\n";
        else if (node->get_nodekind() == NodeKind::ND_NE)
            std::cout << "    setne %al\n";
        else if (node->get_nodekind() == NodeKind::ND_LT)
            std::cout << "    setl %al\n";
        else if (node->get_nodekind() == NodeKind::ND_LE)
            std::cout << "    setle %al\n";

        
        std::cout << "    movzbq %al, %rax\n";  //movzbl will be better
        break;
    default:
        diagnostic::fatal("invalid expression kind in codegen");
    }
}

void CodeGen::gen_stmt(Node* node)
{
    if(node->get_nodekind()==NodeKind::ND_EXPR_STMT)
    {
        gen_expr(node->get_lhs());
        return;
    }
    diagnostic::fatal("invalid statement kind in codegen");
}
