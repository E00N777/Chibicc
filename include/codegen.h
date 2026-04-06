#include "astnode.h"
#include "type.h"
#include <array>
#include <cassert>
#include <string_view>

// Emits x86-64 AT&T asm to stdout. Assumes SysV ABI: args in rdi,rsi,rdx,rcx,r8,r9; return in rax.
// generate(prog) iterates the function list, sets current_fn_ per function, and emits prologue/body/epilogue.
class CodeGen {
public:
    std::array<std::string_view, 6> args_regs = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
    std::array<std::string_view,6> char_args_regs = {"%dil", "%sil", "%dl", "%cl", "%r8b", "%r9b"};
    void generate(Program* program_translation_unit);
    int gen_label_seq() { return label_seq++; }

private:
    int depth = 0;           // Stack depth for expression temporaries (push/pop balance).
    int label_seq = 0;       // Unique suffix for .L.else / .L.end / .L.begin.
    Function* current_fn_ = nullptr;  // Used for ND_RETURN jump target (.L.return.<name>).
    void push();
    void pop(const char* reg);
    void store_function_params(Function* fn);
    void gen_addr(Node* node);
    void gen_expr(Node* node);
    void gen_stmt(Node* node);
    static int align_to(int n, int align);
    void assign_lvar_offsets(Function* prog);
    void load(Type* ty);
    void store(Type* ty);
    void emit_data(Program* program);
};
