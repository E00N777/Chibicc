#pragma once
#include <string_view>

enum class TokenKind{
    PUNCT, //Punctuations
    IDENT, //Identifiers
    NUM, //Numeric literals
    KEYWORD, //Keywords
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

        int get_number() const;
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
        void set_kind(TokenKind kind) {
            this->TKind = kind;
        }
};

// Implement in tokenize.cpp. Input: source code string.
Token* Tokenize(char* Input, const char* filename = "<input>");
Token* Tkskip(Token* TK, const char* op);
bool Tkequal(Token* TK, const char* op);