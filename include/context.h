#pragma once
#include "tokenize.h"
#include "astnode.h"
#include "type.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

// Central ownership for all AST/symbol objects. Parser and codegen hold raw pointers;
// ASTContext keeps unique_ptrs so that everything is freed when ctx goes out of scope.
class ASTContext {
public:
    template<class... Args>
    Token* make_token(Args&&... args) {
        auto p = std::make_unique<Token>(std::forward<Args>(args)...);
        Token* raw = p.get();
        tokens_.push_back(std::move(p));
        return raw;
    }

    template<class... Args>
    Node* make_node(Args&&... args) {
        auto p = std::make_unique<Node>(std::forward<Args>(args)...);
        Node* raw = p.get();
        nodes_.push_back(std::move(p));
        return raw;
    }

    Obj* make_obj(std::string name, Type* ty, Obj* next = nullptr) {
        auto p = std::make_unique<Obj>(std::move(name), ty, next);
        Obj* raw = p.get();
        objs_.push_back(std::move(p));
        return raw;
    }

    template<class... Args>
    Function* make_function(Args&&... args) {
        auto p = std::make_unique<Function>(std::forward<Args>(args)...);
        Function* raw = p.get();
        funcs_.push_back(std::move(p));
        return raw;
    }

    Type* get_int_type() {
        if (!ty_int_) {
            auto p = std::make_unique<Type>(TypeKind::TY_INT);
            p->set_size(8);
            ty_int_ = p.get();
            types_.push_back(std::move(p));
        }
        return ty_int_;
    }

    Type* make_ptr_type(Type* base) {
        auto p = std::make_unique<Type>(TypeKind::TY_PTR, base);
        Type* raw = p.get();
        p->set_size(8);
        types_.push_back(std::move(p));
        return raw;
    }
    Type* make_func_type(Type* return_ty, std::vector<Type*> params = {}) {
        auto p = std::make_unique<Type>(TypeKind::TY_FUNC, return_ty);
        p->set_param_types(std::move(params));
        Type* raw = p.get();
        types_.push_back(std::move(p));
        return raw;
    }
    Type* make_array_type(Type* base, int len) {
        auto p = std::make_unique<Type>(TypeKind::TY_ARRAY);
        p->set_base(base);
        p->set_size(base->get_size() * len);
        p->set_array_len(len);
        Type* raw = p.get();
        types_.push_back(std::move(p));
        return raw;
    }


private:
    std::vector<std::unique_ptr<Token>> tokens_;
    std::vector<std::unique_ptr<Node>> nodes_;
    std::vector<std::unique_ptr<Obj>> objs_;
    std::vector<std::unique_ptr<Function>> funcs_;
    std::vector<std::unique_ptr<Type>> types_;
    Type* ty_int_ = nullptr;
};
