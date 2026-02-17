#include "diagnostic.h"
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

// Fatal error without source location (e.g. invalid arguments).
[[noreturn]] void fatal(const std::string& msg) {
    std::cerr << "error: " << msg << "\n";
    std::exit(1);
}

} // namespace diagnostic
