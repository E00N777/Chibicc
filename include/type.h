#pragma once

class Node;
class ASTContext;

enum class TypeKind { TY_INT, TY_PTR, TY_FUNC };

// Type: int, pointer-to-T, or function-returning-T. Created in parser (declspec/declarator)
// and in context (get_int_type, make_ptr_type, make_func_type). add_type() in type.cpp
// fills Node::ty for expressions; parser uses types for Obj and for +/- pointer arithmetic.
class Type {
public:
    // For TY_PTR extra is base; for TY_FUNC extra is return type; for TY_INT extra ignored.
    explicit Type(TypeKind kind, Type* extra = nullptr)
        : kind_(kind),
          base_(kind == TypeKind::TY_PTR ? extra : nullptr),
          return_ty_(kind == TypeKind::TY_FUNC ? extra : nullptr) {}

    TypeKind get_kind() const { return kind_; }
    Type* get_base() const { return base_; }
    void set_base(Type* base) { base_ = base; }

private:
    TypeKind kind_;
    Type* base_ = nullptr;
    Type* return_ty_ = nullptr;

    Type(const Type&) = delete;
    Type& operator=(const Type&) = delete;
};

bool is_integer(Type* ty, ASTContext& ctx);
Type* get_ty_int(ASTContext& ctx);
// Recursively assign type to every node in the AST (bottom-up). Called from compound_stmt after each stmt.
void add_type(Node* node, ASTContext& ctx);