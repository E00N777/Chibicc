#include "diagnostic.h"
#include "tokenize.h"
#include <iostream>
#include <cstdlib>

namespace diagnostic {

[[noreturn]] void error_at(std::string_view loc, const std::string& msg) {
    std::cerr << "error: " << msg;
    if (!loc.empty()) {
        std::cerr << " ('" << loc << "')";
    }
    std::cerr << "\n";
    std::exit(1);
}

[[noreturn]] void error_tok(Token* tok, const std::string& msg) {
    error_at(tok ? tok->get_content() : std::string_view{}, msg);
}

[[noreturn]] void fatal(const std::string& msg) {
    std::cerr << "error: " << msg << "\n";
    std::exit(1);
}

} // namespace diagnostic
