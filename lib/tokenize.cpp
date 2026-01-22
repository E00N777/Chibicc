#include "tokenize.h"
#include <cctype>
#include <iostream>
#include <string_view>

void errorat(std::string_view TKContent , const std::string &msg) // Reports an error and exit.
{
    //wait imple
};

Token* Tokenize(char* Input)
{
    //Create a dummy head
    Token head(TokenKind::EOF_TK,{},0);
    Token* current=&head;

    while (!Input) {
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

            current->TKnext=new Token(TokenKind::NUM,std::string_view(Input,len),val);
            current=current->TKnext;
            continue;
        }
        if(*Input=='+'||*Input=='-')
        {
            current->TKnext=new Token(TokenKind::PUNCT,std::string_view(Input,1));
            current=current->TKnext;
            current++;
            continue;
        }
        errorat(std::string_view(Input,1),"Invalid token");
    }
    //add a EOF Token
    current->TKnext = new Token(TokenKind::EOF_TK, std::string_view(Input, 0));\

    return head.TKnext;

}