#include "type.h"
#include "astnode.h"
#include "context.h"
#include "diagnostic.h"

Type* get_ty_int(ASTContext& ctx) {
    return ctx.get_int_type();
}

Type* get_ty_char(ASTContext& ctx) {
    return ctx.get_char_type();
}

bool is_integer_or_char(Type* ty, ASTContext& ctx) {
    return ty && (ty->get_kind() == TypeKind::TY_INT || ty->get_kind() == TypeKind::TY_CHAR);
}

// Bottom-up type assignment for the AST. Called from parser (compound_stmt) after each statement.
// Leaves and vars get type from Obj or literal; unary/binary derive from operands.
void add_type(Node* node, ASTContext& ctx) {
    if (!node || node->get_ty())
        return;
    add_type(node->get_lhs(), ctx);
    add_type(node->get_rhs(), ctx);
    add_type(node->get_condition(), ctx);
    add_type(node->get_then(), ctx);
    add_type(node->get_els(), ctx);
    add_type(node->get_init(), ctx);
    add_type(node->get_inc(), ctx);
    for (Node* n = node->get_body(); n; n = n->get_nextstmt())
        add_type(n, ctx);
    for (Node* arg = node->get_args(); arg; arg = arg->get_nextstmt())
        add_type(arg, ctx);
   
    switch (node->get_nodekind()) {
    case NodeKind::ND_GVAR:
        node->set_ty(node->get_gvar()->get_ty());
        return;
    case NodeKind::ND_ADD:
    case NodeKind::ND_SUB:
    case NodeKind::ND_MUL:
    case NodeKind::ND_DIV:
    case NodeKind::ND_NEG:
        node->set_ty(node->get_lhs()->get_ty());
        return;
    case NodeKind::ND_ASSIGN:{
        Type* lhs_ty = node->get_lhs()->get_ty();
        if (lhs_ty && lhs_ty->get_kind() == TypeKind::TY_ARRAY)
            diagnostic::error_tok(node->get_tok(), "invalid array assignment");
        node->set_ty(lhs_ty);
        return;
    }
    case NodeKind::ND_EQ:
    case NodeKind::ND_NE:
    case NodeKind::ND_LT:
    case NodeKind::ND_LE:
    case NodeKind::ND_NUM:
        node->set_ty(get_ty_int(ctx));
        return;
    case NodeKind::ND_VAR:
        node->set_ty(node->get_var()->get_ty());
        return;
    case NodeKind::ND_ADDR: {
        Type* lhs_ty = node->get_lhs()->get_ty();
        if (lhs_ty && lhs_ty->get_kind() == TypeKind::TY_ARRAY)
            node->set_ty(ctx.make_ptr_type(lhs_ty->get_base()));
        else
            node->set_ty(ctx.make_ptr_type(lhs_ty));
        return;
    }      
    case NodeKind::ND_DEREF: {
        Type* lhs_ty = node->get_lhs()->get_ty();
        if (!lhs_ty || !lhs_ty->get_base())
            diagnostic::error_tok(node->get_tok(), "invalid pointer dereference");
        else
            node->set_ty(lhs_ty->get_base());
        return;
    }
    case NodeKind::ND_FUNCALL:
        node->set_ty(get_ty_int(ctx));
        return;
    default:
        return;
    }
}
