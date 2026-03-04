#include "astnode.h"
#include <cassert>
#include <array>
#include <string_view>
class CodeGen {
public:
    //X86-64 calling convention registers
    std::array<std::string_view, 6> args_regs = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
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
