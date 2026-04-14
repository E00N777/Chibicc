#pragma once
#include <string_view>
#include <string>

class ASTContext;
class Type;

enum class TokenKind {
    PUNCT,   // Operators and punctuation: + - * / ( ) { } ; , etc.
    IDENT,   // Identifier (later promoted to KEYWORD if matches keyword)
    NUM,     // Integer literal
    KEYWORD, // if, else, return, for, while, int
    STR, // string literal
    EOF_TK,
};

class Token {
private:
    TokenKind TKind;
    Token* TKnext = nullptr;
    int TKval = 0;  // Numeric value when TKind == NUM
    std::string_view TKContent;

    // string literal
    Type* TKty_ = nullptr;
    std::string TKstr_;

public:
    Token(TokenKind kind, std::string_view content, int val)
        : TKind(kind), TKContent(content), TKval(val) {}
    Token(TokenKind kind, std::string_view content)
        : TKind(kind), TKContent(content) {}

    TokenKind get_kind() const { return TKind; }
    int get_number() const;
    Token* get_next() const { return TKnext; }
    std::string_view get_content() const { return TKContent; }
    void set_next(Token* next) { TKnext = next; }
    void set_kind(TokenKind kind) { TKind = kind; }

    void set_type(Type* ty) { TKty_ = ty; }
    void set_string(std::string str) { TKstr_ = str; }
    Type* get_type() const { return TKty_; }
    std::string get_string() const { return TKstr_; }
};

// Tokenize null-terminated source; tokens are owned by ctx. Returns head of token list.
Token* Tokenize(char* Input, ASTContext& ctx, const char* filename = "<input>");
void Tkskip(Token*& TK, const char* op);
bool Tkequal(Token* TK, const char* op);