#pragma once

#include <string>
#include <string_view>

class Token;

namespace diagnostic {

// Report error at given token/location (no line/column), then exit.
[[noreturn]] void error_at(std::string_view loc, const std::string& msg);

// Report error at token location, then exit.
[[noreturn]] void error_tok(Token* tok, const std::string& msg);

// Fatal error without source location.
[[noreturn]] void fatal(const std::string& msg);

} // namespace diagnostic
