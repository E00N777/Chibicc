#include <iostream>
#include <cstdlib>
#include "tokenize.h"

int main(int argc, char** argv)
{
    if(argc!=2)
    {
        std::cerr<<"[Fatal Error]:Invalid number of arguments\n"<<std::endl;
        std::exit(1);
    }

    Token* token = Tokenize(argv[1]);


    std::cout<<"    .globl main\n";
    std::cout<<"main:\n";
    //The first token must be a number
    if((token->get_kind())!=TokenKind::NUM) {errorat(token->get_content(),"The first token must be a number");}
    std::cout<<"    mov $"<<token->get_number()<<", %rax\n";
    token=token->get_next();

    while ((token->get_kind())!=TokenKind::EOF_TK) {

        if(Tkequal(token,"+"))
        {
            token=Tkskip(token, "+");
            std::cout<<"    add $"<<token->get_number()<<", %rax\n";
            token=token->get_next();
            continue;
        }

        if(Tkequal(token,"-"))
        {
            token=Tkskip(token, "-");
            std::cout<<"    sub $"<<token->get_number()<<", %rax\n";
            token=token->get_next();
            continue;
        }
         std::cerr<<"[Fatal Error]:Invalid operator\n"<<std::endl;
         return 1;
    }
    std::cout<<"    ret\n";

    return 0;

}