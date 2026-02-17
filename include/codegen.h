#include "astnode.h"
#include <iostream>
class CodeGen{
    public:
        void generate(Node* node){
            std::cout<<"    .globl main\n";
            std::cout<<"main:\n";
            for(Node* n=node;n;n=n->get_nextstmt())
            {
                gen_stmt(n);
                if(depth==0){break;}
            }
            std::cout<<"    ret\n";
        }
    private:
        int depth=0;
        void push();
        void pop(const char* reg);
        void gen_expr(Node* node);
        void gen_stmt(Node* node);
};
