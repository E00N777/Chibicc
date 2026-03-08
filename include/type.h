#pragma once

class Node;
class ASTContext;

enum class TypeKind {
    TY_INT,
    TY_PTR,
    TY_FUNC,
};

// Represents a type: int or pointer-to-T.
class Type {
public:
    // One ctor: second arg is base type for TY_PTR, return type for TY_FUNC.
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
    //Function type
    Type* return_ty_=nullptr;

    Type(const Type&) = delete;
    Type& operator=(const Type&) = delete;
};

bool is_integer(Type* ty,ASTContext& ctx);
// Returns the singleton int type (creates it on first use).
Type* get_ty_int(ASTContext& ctx);
void add_type(Node* node,ASTContext& ctx);