#include "codegen.h"
#include "context.h"
#include "parser.h"
#include "tokenize.h"
#include <cstdlib>
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "error: expected exactly one argument (source string)\n";
        std::exit(1);
    }
    ASTContext ctx;
    Token* token = Tokenize(argv[1], ctx);
    Parser parser(token, ctx);
    Program* program_translation_unit = parser.parse();
    CodeGen codegen;
    codegen.generate(program_translation_unit);
    return 0;
}