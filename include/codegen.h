#include "astnode.h"
#include <cassert>
#include <iostream>

class CodeGen {
public:
    void generate(Function* prog);
    int gen_label_seq(){return label_seq++;};

private:
    int depth = 0;
    int label_seq = 0;
    void push();
    void pop(const char* reg);
    void gen_addr(Node* node);
    void gen_expr(Node* node);
    void gen_stmt(Node* node);
    static int align_to(int n, int align);
    void assign_lvar_offsets(Function* prog);
};
