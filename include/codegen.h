#include "astnode.h"

class CodeGen{
    public:
        void generate(Node* node){
            gen_expr(node);
        }
    private:
        int depth=0;
        void push();
        void pop(const char* reg);
        void gen_expr(Node* node);//key func
};
