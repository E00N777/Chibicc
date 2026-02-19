#include "tokenize.h"
#include "diagnostic.h"
#include <cctype>
#include <string_view>
#include <cstring>


static const char* multi_char_ops[]={"==","!=",">=","<="};



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

Token* Tkskip(Token* TK, const char* op)
{
    if (!Tkequal(TK, op)) {
        diagnostic::error_at(TK->get_content(), "expected '" + std::string(op) + "'");
    }
    return TK->get_next();
}

static bool startswith(const char* p, const char* keyword) {
    return std::strncmp(p, keyword, strlen(keyword)) == 0;
}

// Returns true if c is valid as the first character of an identifier.
static bool is_ident1(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

// Returns true if c is valid as a non-first character of an identifier.
static bool is_ident2(char c) {
    return is_ident1(c) || ('0' <= c && c <= '9');
}

Token* Tokenize(char* Input, const char* filename) {
    (void)filename;
    // Create a dummy head
    Token head(TokenKind::EOF_TK,{},0);
    Token* current=&head;

    while (*Input) {
        // Skip whitespace
        if (std::isspace(*Input))
        {
            Input++;
            continue;      
        }

        if(std::isdigit(*Input))
        {
            char* temp=Input;
            int val=std::strtol(Input,&Input,10);
            int len=Input-temp;

            Token* new_token=new Token(TokenKind::NUM,std::string_view(temp,len),val);
            current->set_next(new_token);
            current=current->get_next();
            continue;
        }
        // Identifiers (multi-character: letters, digits, underscore)
        if (is_ident1(*Input)) {
            char* start = Input;
            do { Input++; } while (is_ident2(*Input));
            Token* new_token = new Token(TokenKind::IDENT, std::string_view(start, Input - start));
            current->set_next(new_token);
            current = current->get_next();
            continue;
        }

        bool found_multi_ops = false;
        for(const char* op : multi_char_ops)
        {
            
            if(startswith(Input,op))
            {
                Token* new_token=new Token(TokenKind::PUNCT,std::string_view(Input,2));
                current->set_next(new_token);
                current=current->get_next();
                Input += 2;
                found_multi_ops=true;
                break;
            }
        }
        if(found_multi_ops){continue;}

        if(std::ispunct(*Input))
        {
            Token* new_token=new Token(TokenKind::PUNCT,std::string_view(Input,1));
            current->set_next(new_token);
            current=current->get_next();
            Input++;
            continue;
        }
        diagnostic::error_at(std::string_view(Input, 1), "invalid token");
    }
    // Append EOF token
    Token* eof_token=new Token(TokenKind::EOF_TK, std::string_view(Input, 0));
    current->set_next(eof_token);

    return head.get_next();

}