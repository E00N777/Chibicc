#pragma once
//Node for AST
enum class NodeKind
{
    ND_ADD, //+
    ND_SUB, //-
    ND_MUL, //*
    ND_DIV, // /
    ND_NUM, //integer
};

class Node{
    private:
        NodeKind kind;
        Node* lhs = nullptr; // lift hand side
        Node* rhs = nullptr; // right hand side
        int val = 0;
    public:
        Node(NodeKind kind): kind(kind){};
        Node(NodeKind kind,Node* lhs,Node* rhs):kind(kind),lhs(lhs),rhs(rhs){};
        Node(int val):val(val){this->kind=NodeKind::ND_NUM;}
        //================Getter====================
        NodeKind get_nodekind()
        {
            return this->kind;
        }
        int get_val()
        {
            return this->val;
        }
        Node* get_lhs()
        {
            return this->lhs;
        }
        Node* get_rhs()
        {
            return this->rhs;
        }

    
};