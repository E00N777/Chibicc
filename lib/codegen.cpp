#include "codegen.h"
#include "astnode.h"
#include "diagnostic.h"
#include <iostream>

// Round up n to the nearest multiple of align (e.g. 5,8 -> 8; 11,8 -> 16).
int CodeGen::align_to(int n, int align) {
    return (n + align - 1) / align * align;
}

void CodeGen::load(Type* ty) {
    if (ty->get_kind() == TypeKind::TY_ARRAY) {
        return;
    }
    std::cout << "    mov (%rax), %rax\n";
}

void CodeGen::push() {
    std::cout << "    push %rax\n";
    depth++;
}
void CodeGen::pop(const char* reg) {
    std::cout << "    pop " << reg << "\n";
    depth--;
}

void CodeGen::emit_data(Program* program) {
    for(GlobalVariable* gv = program->get_global_variable_begin(); gv; gv = gv->get_next()) {
        std::cout << "    .data\n";
        std::cout << ".globl " << gv->get_name() << "\n";
        std::cout << gv->get_name() << ":\n";
        std::cout << "    .zero " << gv->get_ty()->get_size() << "\n";
    }
}

// The SysV ABI passes the first six integer arguments in registers. We spill
// them into the stack slots assigned to the function's parameter variables so
// the rest of the compiler can treat parameters exactly like ordinary locals.
void CodeGen::store_function_params(Function* fn) {
    if (fn->get_params().size() > args_regs.size())
        diagnostic::error_at(fn->get_name(), "too many function parameters");

    for (std::size_t i = 0; i < fn->get_params().size(); ++i) {
        LocalVariable* param = fn->get_params()[i];
        std::cout << "    mov " << args_regs[i] << ", " << param->get_offset() << "(%rbp)\n";
    }
}

// Emit code to leave the address of the node in %rax. Valid for ND_VAR, ND_DEREF, ND_ADDR.
void CodeGen::gen_addr(Node* node) {
    switch(node->get_nodekind())
    {
        case NodeKind::ND_VAR:
            std::cout << "    lea " << node->get_var()->get_offset() << "(%rbp), %rax\n";
            break;
        case NodeKind::ND_GVAR:
            std::cout << "    lea " << node->get_gvar()->get_name() << "(%rip), %rax\n";
            break;
        case NodeKind::ND_DEREF:
            gen_expr(node->get_lhs());
            return;
        case NodeKind::ND_ADDR:
            gen_addr(node->get_lhs());
            return;
        default:
            diagnostic::error_tok(node->get_tok(), "lvalue required for address-of or dereference");
    }
}

// Set each function's local var offsets and stack_size (16-byte aligned).
void CodeGen::assign_lvar_offsets(Function* prog) {
    for (Function* fn = prog; fn; fn = fn->get_next()) {
        int offset = 0;
        for (LocalVariable* var = fn->get_locals(); var; var = var->get_next()) {
            offset += var->get_ty()->get_size();
            var->set_offset(-offset);
        }
        fn->set_stack_size(align_to(offset, 16));
    }
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
    case NodeKind::ND_GVAR:
        gen_addr(node);
        load(node->get_ty());
        return;
    case NodeKind::ND_ASSIGN:
        gen_addr(node->get_lhs());
        push();
        gen_expr(node->get_rhs());
        pop("%rdi");
        std::cout << "    mov %rax, (%rdi)\n";
        return;
    case NodeKind::ND_ADDR:
        gen_addr(node->get_lhs());
        return;
    case NodeKind::ND_DEREF:
        gen_addr(node);
        load(node->get_ty());
        return;
    case NodeKind::ND_FUNCALL:
        int arg_count = 0;
        for(Node* arg=node->get_args();arg;arg=arg->get_nextstmt())
        {
            gen_expr(arg);
            push();
            arg_count++;
           
        }
        for(int i = arg_count-1;i>=0;i--)
        {
            pop(args_regs[i].data());
        }
        std::cout << "    call " << node->get_func_name() << "\n";
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

        
        std::cout << "    movzbq %al, %rax\n";
        break;
    default:
        diagnostic::error_tok(node->get_tok(), "invalid expression");
    }
}

void CodeGen::gen_stmt(Node* node) {
    switch(node->get_nodekind())
    {
        case NodeKind::ND_IF: {
            int seq=gen_label_seq();
            gen_expr(node->get_condition());
            std::cout << "    cmp $0, %rax\n";
            std::cout << "    je .L.else" << seq << "\n";
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
            if(node->get_init()){
                gen_stmt(node->get_init());
            }
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
            std::cout << "    jmp .L.return."<<current_fn_->get_name()<<"\n";
            return;
        case NodeKind::ND_BLOCK:
            for (Node* n = node->get_body(); n; n = n->get_nextstmt()) {
                gen_stmt(n);
            }
            return;
        default:
            diagnostic::error_tok(node->get_tok(), "invalid statement");
    }
    diagnostic::error_tok(node->get_tok(), "invalid statement");
}

void CodeGen::generate(Program* program_translation_unit) {

    emit_data(program_translation_unit);
    std::cout << "    .text\n";
    assign_lvar_offsets(program_translation_unit->get_function_begin());
    
    for (Function* fn = program_translation_unit->get_function_begin(); fn; fn = fn->get_next()) {
        current_fn_ = fn;
        std::cout << "    .globl " << fn->get_name() << "\n";
        std::cout << fn->get_name() << ":\n";
        std::cout << "    push %rbp\n";
        std::cout << "    mov %rsp, %rbp\n";
        std::cout << "    sub $" << fn->get_stack_size() << ", %rsp\n";
        store_function_params(fn);

        for (Node* n = fn->get_body(); n; n = n->get_nextstmt()) {
            gen_stmt(n);
            assert(depth == 0);
        }

        std::cout << ".L.return." << fn->get_name() << ":\n";
        std::cout << "    mov %rbp, %rsp\n";
        std::cout << "    pop %rbp\n";
        std::cout << "    ret\n";
        }


    
}
