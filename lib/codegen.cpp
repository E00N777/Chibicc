#include "codegen.h"
#include "astnode.h"
#include "diagnostic.h"
#include <iostream>

// Round up n to the nearest multiple of align. E.g. align_to(5, 8) => 8, align_to(11, 8) => 16.
int CodeGen::align_to(int n, int align) {
    return (n + align - 1) / align * align;
}

void CodeGen::push() {
    std::cout << "    push %rax\n";
    depth++;
}
void CodeGen::pop(const char* reg) {
    std::cout << "    pop " << reg << "\n";
    depth--;
}

// Compute the absolute address of a given node.
// It's an error if the node does not reside in memory.
void CodeGen::gen_addr(Node* node) {
    if (node->get_nodekind() == NodeKind::ND_VAR) {
        std::cout << "    lea " << node->get_var()->get_offset() << "(%rbp), %rax\n";
        return;
    }
    diagnostic::fatal("not an lvalue");
}

// Assign offsets to local variables.
void CodeGen::assign_lvar_offsets(Function* prog) {
    int offset = 0;
    for (Obj* var = prog->get_locals(); var; var = var->get_next()) {
        offset += 8;
        var->set_offset(-offset);
    }
    prog->set_stack_size(align_to(offset, 16));
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
    case NodeKind::ND_VAR:
        gen_addr(node);
        std::cout << "    mov (%rax), %rax\n";
        return;
    case NodeKind::ND_ASSIGN:
        gen_addr(node->get_lhs());
        push();
        gen_expr(node->get_rhs());
        pop("%rdi");
        std::cout << "    mov %rax, (%rdi)\n";
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

void CodeGen::gen_stmt(Node* node) {
    switch(node->get_nodekind())
    {
        case NodeKind::ND_IF: {
            int seq=gen_label_seq();
            gen_expr(node->get_condition());
            std::cout << "    cmp $0, %rax\n";
            std::cout << "     je .L.else" << seq << "\n";
            gen_stmt(node->get_then());
            std::cout << "    jmp .L.end" << seq << "\n";
            std::cout << ".L.else" << seq << ":\n";
            if(node->get_els())
            {
                gen_stmt(node->get_els());
            }
            std::cout << ".L.end" << seq << ":\n";
            return;
        }
        case NodeKind::ND_FOR:{
            int seq=gen_label_seq();
            gen_stmt(node->get_init());
            std::cout <<".L.begin" << seq << ":\n";
            if(node->get_condition())
            {
                gen_expr(node->get_condition());
                std::cout << "    cmp $0, %rax\n";
                std::cout << "    je .L.end" << seq << "\n";
            }
            gen_stmt(node->get_then());
            if(node->get_inc())
            {
                gen_expr(node->get_inc());
            }
            std::cout << "    jmp .L.begin" << seq << "\n";
            std::cout << ".L.end" << seq << ":\n";
            return;
        }
        case NodeKind::ND_EXPR_STMT:
            gen_expr(node->get_lhs());
            return;
        case NodeKind::ND_RETURN:
            gen_expr(node->get_lhs());
            std::cout << "    jmp .L.return\n";
            return;
        case NodeKind::ND_BLOCK:
            for (Node* n = node->get_body(); n; n = n->get_nextstmt()) {
                gen_stmt(n);
            }
            return;
        default:
            diagnostic::fatal("invalid statement kind in codegen");
    }
    diagnostic::fatal("invalid statement kind in codegen");
}

void CodeGen::generate(Function* prog) {
    assign_lvar_offsets(prog);

    std::cout << "    .globl main\n";
    std::cout << "main:\n";
    // Prologue
    std::cout << "    push %rbp\n";
    std::cout << "    mov %rsp, %rbp\n";
    std::cout << "    sub $" << prog->get_stack_size() << ", %rsp\n";

    for (Node* n = prog->get_body(); n; n = n->get_nextstmt()) {
        gen_stmt(n);
        assert(depth == 0);
    }

    std::cout << ".L.return:\n";
    std::cout << "    mov %rbp, %rsp\n";
    std::cout << "    pop %rbp\n";
    std::cout << "    ret\n";
}
