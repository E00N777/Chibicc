#pragma once
//Node for AST
enum class NodeKind
{
    ND_ADD, //+
    ND_SUB, //-
    ND_MUL, //*
    ND_DIV, // /
    ND_NUM, //integer
    ND_NEG, // negative  -   we don't need explicitly handle positive + cuz +(10)=10
    ND_EQ, // ==
    ND_NE, // !=
    ND_LT, // < less than 
    ND_LE, // <= less than or equal to
    ND_EXPR_STMT, // expression for statement
};

class Node{
    private:
        NodeKind kind;
        Node* lhs = nullptr; // lift hand side
        Node* rhs = nullptr; // right hand side
        Node* next = nullptr; // Next node
        int val = 0;
    public:
        Node(NodeKind kind): kind(kind){};
        Node(NodeKind kind,Node* lhs,Node* rhs):kind(kind),lhs(lhs),rhs(rhs){};
        Node(NodeKind kind,Node* lhs):kind(kind),lhs(lhs){};
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
        Node* get_nextstmt()
        {
            return this->next;
        }
        //=======setter func========
        void set_nextstmt(Node* nextstmt)
        {
            this->next=nextstmt;
        }

    
};