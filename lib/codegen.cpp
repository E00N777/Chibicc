#include <iostream>
#include "codegen.h"
#include "astnode.h"


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
    default:
        std::cerr << "Error: invalid expression kind\n";
        exit(1);
    }
}
