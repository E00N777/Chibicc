#include "tokenize.h"
#include <cctype>
#include <iostream>
#include <string_view>


bool Tkequal(Token* TK,const char* op)
{
    if (!TK || TK->get_content().empty()) return false;
    return TK->get_content() == op;
}


void errorat(std::string_view TKContent , const std::string &msg) // Reports an error and exit.
{
    
    std::cout << "[ERROR] " << msg << ": \"" << TKContent << "\"" << std::endl;
    std::exit(1);

};

Token* Tkskip(Token* TK, const char* op)
{
    if(!Tkequal(TK,op))
    {
        errorat(TK->get_content(), "expected '" + std::string(op) + "'");
    }
    return TK->get_next();
}

Token* Tokenize(char* Input)
{
    //Create a dummy head
    Token head(TokenKind::EOF_TK,{},0);
    Token* current=&head;

    while (*Input) {
        //sikp whitespace char
        if(std::isspace(*Input))
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
        if(std::ispunct(*Input))
        {
            Token* new_token=new Token(TokenKind::PUNCT,std::string_view(Input,1));
            current->set_next(new_token);
            current=current->get_next();
            Input++;
            continue;
        }
        errorat(std::string_view(Input,1),"Invalid token");
    }
    //add a EOF Token
    Token* eof_token=new Token(TokenKind::EOF_TK, std::string_view(Input, 0));
    current->set_next(eof_token);

    return head.get_next();

}