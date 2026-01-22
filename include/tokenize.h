#pragma once
#include <string_view>
#include <string>

enum class TokenKind{
    PUNCT, //Punctuations
    NUM, //Numeric literals
    EOF_TK, //End-of-file markers
};
class Token{
    public:
        TokenKind TKind; // Token Kind
        Token* TKnext = nullptr; //next Token
        int TKval=0; // If the Token is Token_NUM
        std::string_view TKContent; //Replace loc and len in C style

        //constructor for num
        Token(TokenKind kind, std::string_view content, int val)
            : TKind(kind),TKContent(content),TKval(val){}; 
        //constructor for operator and EOF
        Token(TokenKind kind, std::string_view content)
            : TKind(kind),TKContent(content){};  
             
        bool equal(std::string_view op) const;
        Token* skip(std::string_view op) const;
        int get_number() const;     
};  