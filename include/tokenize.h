#pragma once
#include <string_view>
#include <iostream>
#include <string>

enum class TokenKind{
    PUNCT, //Punctuations
    NUM, //Numeric literals
    EOF_TK, //End-of-file markers
};
class Token{
    private:
        TokenKind TKind; // Token Kind
        Token* TKnext = nullptr; //next Token
        int TKval=0; // If the Token is Token_NUM
        std::string_view TKContent; //Replace loc and len in C style
    public:
        //constructor for num
        Token(TokenKind kind, std::string_view content, int val)
            : TKind(kind),TKContent(content),TKval(val){}; 
        //constructor for operator and EOF
        Token(TokenKind kind, std::string_view content)
            : TKind(kind),TKContent(content){};
            
        // --- Getter Methods ---
        TokenKind get_kind() const { return TKind; }

        int get_number() const
        {
            if(this->TKind != TokenKind::NUM)
            {
                std::cout<<"[ERROR]:This token is not a NUM\n";
                std::exit(1);
            }
            return this->TKval;
        }
        Token* get_next() const{
            return this->TKnext;
        }
        std::string_view get_content() const{
            return this->TKContent;
        }

        // --- Setter Methods ---
        void set_next(Token* next) {
        this->TKnext = next;
    }     
};

//Implement in tokenize.cpp
Token* Tokenize(char* Input);
Token* Tkskip(Token* TK, const char* op);
[[noreturn]] void errorat(std::string_view TKContent , const std::string &msg);
bool Tkequal(Token* TK,const char* op);