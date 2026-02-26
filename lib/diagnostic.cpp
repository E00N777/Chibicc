#include "diagnostic.h"
#include "tokenize.h"
#include <iostream>
#include <cstdlib>

namespace diagnostic {

// Report error at given token/location and exit.
[[noreturn]] void error_at(std::string_view loc, const std::string& msg) {
    std::cerr << "error: " << msg;
    if (!loc.empty()) {
        std::cerr << " ('" << loc << "')";
    }
    std::cerr << "\n";
    std::exit(1);
}

// Report error at token location and exit.
[[noreturn]] void error_tok(Token* tok, const std::string& msg) {
    error_at(tok ? tok->get_content() : std::string_view{}, msg);
}

// Fatal error without source location (e.g. invalid arguments).
[[noreturn]] void fatal(const std::string& msg) {
    std::cerr << "error: " << msg << "\n";
    std::exit(1);
}

} // namespace diagnostic
