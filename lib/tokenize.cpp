#include "tokenize.h"
#include "diagnostic.h"
#include "context.h"
#include <array>
#include <cctype>
#include <cstring>
#include <string_view>

static constexpr std::array<std::string_view, 4> multi_char_ops = {"==", "!=", ">=", "<="};
static constexpr std::array<std::string_view, 8> keywords = {"if", "else", "return", "for", "while", "int","sizeof","char"};


bool Tkequal(Token* TK,const char* op)
{
    if (!TK || TK->get_content().empty()) return false;
    return TK->get_content() == op;
}


int Token::get_number() const {
    if (this->TKind != TokenKind::NUM) {
        diagnostic::error_at(this->TKContent, "expected a number");
    }
    return this->TKval;
}

void Tkskip(Token*& TK, const char* op)
{
    if (!Tkequal(TK, op)) {
        diagnostic::error_at(TK->get_content(), "expected '" + std::string(op) + "'");
    }
    TK = TK->get_next();
}

static bool if_keyword(std::string_view tok_content)
{
    for(const std::string_view keyword : keywords)
    {
        if(tok_content == keyword)
        {
            return true;
        }
    }
    return false;
}

static Token* read_string_literal(char* &p, ASTContext& ctx) {
    char* start = p;
    char * q = start + 1;
    while(*q != '"') {
        if(*q == '\0' || *q == '\n') {
            diagnostic::error_at(std::string_view(start, q - start), "unterminated string literal");
        }
        q++;
    }
    int body_len = static_cast<int>(q - (start + 1));
    int token_len = static_cast<int>(q - start) + 1;

    Token* new_token = ctx.make_token(TokenKind::STR, std::string_view(start, token_len));

    std::string bytes(start + 1, body_len);
    bytes.push_back('\0');
    new_token->set_string(std::move(bytes));

    Type* arr_type = ctx.make_array_type(ctx.get_char_type(), body_len+1);
    new_token->set_type(arr_type);

    p = start + token_len;
    return new_token;
}

static void convert_keyword(Token* TK)
{
    while(TK->get_kind()!=TokenKind::EOF_TK){
    if(if_keyword(TK->get_content()) && TK->get_kind() == TokenKind::IDENT)
    {
        TK->set_kind(TokenKind::KEYWORD);
    }
    TK=TK->get_next();
    }

}

static bool starts_with(const char* p, std::string_view prefix) {
    for (std::size_t i = 0; i < prefix.size(); ++i) {
        if (p[i] == '\0') return false;
    }
    return std::string_view(p, prefix.size()) == prefix;
}

// Returns true if c is valid as the first character of an identifier.
static bool is_ident1(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

// Returns true if c is valid as a non-first character of an identifier.
static bool is_ident2(char c) {
    return is_ident1(c) || ('0' <= c && c <= '9');
}

Token* Tokenize(char* Input, ASTContext& ctx, const char* filename) {
    (void)filename;
    Token head(TokenKind::EOF_TK, {}, 0);
    Token* current = &head;

    while (*Input) {
        if (std::isspace(*Input))
        {
            Input++;
            continue;
        }

        if (*Input == '"') {
            Token* new_token = read_string_literal(Input, ctx);
            current->set_next(new_token);
            current = current->get_next();
            continue;                   // Input 已经被函数推进
        }

        if (std::isdigit(*Input)) {
            char* temp = Input;
            int val = std::strtol(Input, &Input, 10);
            int len = static_cast<int>(Input - temp);
            Token* new_token = ctx.make_token(TokenKind::NUM, std::string_view(temp, len), val);
            current->set_next(new_token);
            current = current->get_next();
            continue;
        }
        if (is_ident1(*Input)) {
            char* start = Input;
            do { Input++; } while (is_ident2(*Input));
            Token* new_token = ctx.make_token(TokenKind::IDENT, std::string_view(start, Input - start));
            current->set_next(new_token);
            current = current->get_next();
            continue;
        }

        bool found_multi_ops = false;
        for (std::string_view op : multi_char_ops) {
            if (starts_with(Input, op)) {
                Token* new_token = ctx.make_token(TokenKind::PUNCT, std::string_view(Input, op.size()));
                current->set_next(new_token);
                current = current->get_next();
                Input += op.size();
                found_multi_ops = true;
                break;
            }
        }
        if (found_multi_ops) continue;

        if (std::ispunct(*Input)) {
            Token* new_token = ctx.make_token(TokenKind::PUNCT, std::string_view(Input, 1));
            current->set_next(new_token);
            current = current->get_next();
            Input++;
            continue;
        }
        diagnostic::error_at(std::string_view(Input, 1), "invalid token");
    }
    Token* eof_token = ctx.make_token(TokenKind::EOF_TK, std::string_view(Input, 0));
    current->set_next(eof_token);
    convert_keyword(head.get_next());
    return head.get_next();
}
