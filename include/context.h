#pragma once
#include "tokenize.h"
#include "astnode.h"
#include "type.h"

#include <vector>
#include <memory>

class ASTContext{
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

        Type* get_int_type() {
            if (!ty_int_) {
                auto p = std::make_unique<Type>(TypeKind::TY_INT);
                ty_int_ = p.get();
                types_.push_back(std::move(p));
            }
            return ty_int_;
        }
    private:
        std::vector<std::unique_ptr<Token>>  tokens_ ;
        std::vector<std::unique_ptr<Node>>  nodes_;
        std::vector<std::unique_ptr<Obj>>   objs_;
        std::vector<std::unique_ptr<Type>>  types_;
        Type* ty_int_ = nullptr;
};
