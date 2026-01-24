#include <iostream>
#include <cstdlib>
#include "codegen.h"
#include "tokenize.h"
#include "parser.h"
#include <assert.h>

int main(int argc, char** argv)
{
    if(argc!=2)
    {
        std::cerr<<"[Fatal Error]:Invalid number of arguments\n"<<std::endl;
        std::exit(1);
    }

    Token* token = Tokenize(argv[1]);
    Parser parse(token);
    Node* node = parse.parse();

    std::cout<<"    .globl main\n";
    std::cout<<"main:\n";
    
    CodeGen codegen;
    codegen.generate(node);

    std::cout<<"    ret\n";
    return 0;

}