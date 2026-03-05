#include <iostream>
#include <cstdlib>
#include "codegen.h"
#include "tokenize.h"
#include "parser.h"
#include "context.h"

#include <assert.h>

int main(int argc, char** argv)
{
    if(argc!=2)
    {
        std::cerr<<"[Fatal Error]:Invalid number of arguments\n"<<std::endl;
        std::exit(1);
    }
    ASTContext ctx;
    Token* token = Tokenize(argv[1],ctx);
    Parser parser(token,ctx);
    Function* prog = parser.parse();

    CodeGen codegen;
    codegen.generate(prog);
    return 0;

}