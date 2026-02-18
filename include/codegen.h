#include "astnode.h"
#include <iostream>
#include <cassert>
class CodeGen{
    public:
        void generate(Node* node){
            std::cout<<"    .globl main\n";
            std::cout<<"main:\n";
            // Prologue
            std::cout<<"    push %rbp\n";
            std::cout<<"    mov %rsp, %rbp\n";
            std::cout<<"    sub $208, %rsp\n";
            for(Node* n=node;n;n=n->get_nextstmt())
            {
                gen_stmt(n);
                assert(depth==0);
            }
            std::cout<<"    mov %rbp, %rsp\n";
            std::cout<<"    pop %rbp\n";
            std::cout<<"    ret\n";
        }
    private:
        int depth=0;
        void push();
        void pop(const char* reg);
        void gen_addr(Node* node);
        void gen_expr(Node* node);
        void gen_stmt(Node* node);
};
