#include "astnode.h"
#include "tokenize.h"

class Parser{
    private:
        //A pointer for the token processing
        Token* current;

        //Implement in parser.cpp
        Node* primary();
        Node* mul();
        Node* expr();
        Node* unary();
        Node* equality();
        Node* relational();
        Node* add();
    public:
        Parser(Token* tk):current(tk){};

        Node* parse() {
            return expr();
        }

};
