#pragma once

class Node;

enum class TypeKind {
    TY_INT,
    TY_PTR,
};

// Represents a type: int or pointer-to-T.
class Type {
public:
    explicit Type(TypeKind kind, Type* base = nullptr) : kind_(kind), base_(base) {}

    TypeKind get_kind() const { return kind_; }
    Type* get_base() const { return base_; }
    void set_base(Type* base) { base_ = base; }

    static Type* ty_int;
    static Type* pointer_to(Type* base);

private:
    TypeKind kind_;
    Type* base_ = nullptr;

    Type(const Type&) = delete;
    Type& operator=(const Type&) = delete;
};

bool is_integer(Type* ty);
// Returns the singleton int type (creates it on first use).
Type* get_ty_int();
void add_type(Node* node);