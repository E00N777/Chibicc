#include "type.h"
#include "astnode.h"

Type* Type::ty_int = nullptr;

Type* get_ty_int() {
    if (!Type::ty_int)
        Type::ty_int = new Type(TypeKind::TY_INT);
    return Type::ty_int;
}

Type* Type::pointer_to(Type* base) {
    return new Type(TypeKind::TY_PTR, base);
}

bool is_integer(Type* ty) {
    return ty && ty->get_kind() == TypeKind::TY_INT;
}

// Recursively assign type to each AST node (bottom-up).
void add_type(Node* node) {
    if (!node || node->get_ty())
        return;

    add_type(node->get_lhs());
    add_type(node->get_rhs());
    add_type(node->get_condition());
    add_type(node->get_then());
    add_type(node->get_els());
    add_type(node->get_init());
    add_type(node->get_inc());

    for (Node* n = node->get_body(); n; n = n->get_nextstmt())
        add_type(n);

    switch (node->get_nodekind()) {
    case NodeKind::ND_ADD:
    case NodeKind::ND_SUB:
    case NodeKind::ND_MUL:
    case NodeKind::ND_DIV:
    case NodeKind::ND_NEG:
    case NodeKind::ND_ASSIGN:
        node->set_ty(node->get_lhs()->get_ty());
        return;
    case NodeKind::ND_EQ:
    case NodeKind::ND_NE:
    case NodeKind::ND_LT:
    case NodeKind::ND_LE:
    case NodeKind::ND_VAR:
    case NodeKind::ND_NUM:
        node->set_ty(get_ty_int());
        return;
    case NodeKind::ND_ADDR:
        node->set_ty(Type::pointer_to(node->get_lhs()->get_ty()));
        return;
    case NodeKind::ND_DEREF: {
        Type* lhs_ty = node->get_lhs()->get_ty();
        if (lhs_ty->get_kind() == TypeKind::TY_PTR)
            node->set_ty(lhs_ty->get_base());
        else
            node->set_ty(get_ty_int());
        return;
    }
    default:
        return;
    }
}
