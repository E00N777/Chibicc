# Casting

Casting is a small C compiler written in modern C++20. It takes a C translation unit passed as a single command-line string, parses it into an AST, and emits x86-64 AT&T assembly for Linux (System V ABI).

The project started as a C++ rewrite of [chibicc](https://github.com/rui314/chibicc), but the implementation has already grown beyond the original "single function body" stage. The current codebase supports multiple function definitions, global variables, arrays, `char`, and `sizeof`, while keeping the implementation compact enough to study end to end.

This repository is best read as an educational compiler project: tokenizer -> parser -> type propagation -> code generation.

## Current Status

The implementation and the test suite are currently aligned around the following feature set:

- Integer and character types: `int`, `char`
- Pointer types and pointer arithmetic
- Fixed-size arrays, including multi-dimensional arrays
- Local variables and global variables
- Global integer initializers with constant expressions
- Function definitions inside the input translation unit
- Function calls, including recursive calls
- Control flow: `if` / `else`, `while`, `for`, `return`, blocks
- Unary operators: `+`, `-`, `&`, `*`, `sizeof`
- Binary operators: `+`, `-`, `*`, `/`, `==`, `!=`, `<`, `<=`, `>`, `>=`, `=`
- Array indexing syntax: `a[i]` and even `i[a]`
- x86-64 code generation targeting Linux System V ABI

The bundled integration test currently passes locally with `ctest`, covering arithmetic, locals, globals, function calls, recursion, arrays, `char`, and `sizeof`.

## Supported C Subset

Casting currently accepts source shaped like a small translation unit, for example:

```c
int x = 3;

int add2(int a, int b) {
  return a + b;
}

int main() {
  int y[3];
  y[0] = x;
  y[1] = 4;
  y[2] = add2(y[0], y[1]);
  return y[2];
}
```

### Syntax and semantics implemented

- Top-level declarations
  - Function definitions
  - Global variable declarations
  - Global integer initializers using constant expressions
- Statements
  - Expression statements
  - Compound blocks `{ ... }`
  - `if` / `else`
  - `while`
  - `for`
  - `return`
- Expressions
  - Integer literals
  - Variable references
  - Assignment
  - Arithmetic and comparison operators
  - Address-of and dereference
  - Function calls with up to 6 register-passed arguments
  - Array subscripting
  - `sizeof`
- Types
  - `int`
  - `char`
  - Pointers to supported types
  - Fixed-size arrays
  - Function types in declarations

### Important limitations

- Input is passed as one command-line string, not from `.c` files yet.
- Only `int` and `char` scalar types are implemented.
- No string literals, structs, unions, typedefs, enums, or preprocessor.
- No local initializer lists or aggregate initialization.
- Global initializers are limited to constant expressions and only for `int` globals.
- No explicit function prototype handling.
- Code generation assumes Linux x86-64 and the System V calling convention.
- The backend currently supports up to 6 function arguments in registers.
- Block syntax is supported, but the compiler does not model full C scope machinery yet.

## Architecture

```text
Source string
    |
    v
Tokenizer
    |
    v
Recursive-descent parser
    |
    v
AST + symbols + types
    |
    v
Type propagation
    |
    v
Code generator
    |
    v
x86-64 AT&T assembly
```

### Main components

- [`Main.cpp`](Main.cpp)
  - Entry point that tokenizes, parses, and emits assembly.
- [`include/tokenize.h`](include/tokenize.h) / [`lib/tokenize.cpp`](lib/tokenize.cpp)
  - Lexer for identifiers, keywords, integers, punctuation, and multi-character operators.
- [`include/parser.h`](include/parser.h) / [`lib/parser.cpp`](lib/parser.cpp)
  - Recursive-descent parser for declarations, statements, expressions, arrays, functions, globals, and `sizeof`.
- [`include/type.h`](include/type.h) / [`lib/type.cpp`](lib/type.cpp)
  - Type objects plus bottom-up AST type propagation.
- [`include/codegen.h`](include/codegen.h) / [`lib/codegen.cpp`](lib/codegen.cpp)
  - x86-64 assembly emitter.
- [`include/context.h`](include/context.h)
  - Central ownership arena for tokens, nodes, types, functions, and globals.

## Build

Requirements:

- CMake 3.10+
- A C++20 compiler
- GCC toolchain available for linking test binaries from generated assembly

```bash
git clone https://github.com/E00N777/Casting.git
cd Casting
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

## Run

Casting expects exactly one argument: the entire C translation unit.

```bash
./build/Casting 'int main() { return 42; }' > tmp.s
gcc -static -o tmp tmp.s
./tmp
echo $?
```

Example with globals and multiple functions:

```bash
./build/Casting '
int base = 3;

int add2(int a, int b) {
  return a + b;
}

int main() {
  return add2(base, 5);
}' > tmp.s

gcc -static -o tmp tmp.s
./tmp
echo $?
```

## Test

The repository includes a shell-based integration test suite wired into CTest.

```bash
cd build
ctest --output-on-failure
```

Or run the script directly:

```bash
./test.sh ./build/Casting
```

The current test coverage includes:

- Arithmetic and comparisons
- Local variables and assignments
- Control flow
- Pointers and pointer arithmetic
- Arrays and multi-dimensional arrays
- `sizeof`
- Global variables and global initializers
- Function definitions, parameter passing, and recursion
- `char` values and `char` parameters

## Project Layout

```text
Casting/
├── Main.cpp
├── CMakeLists.txt
├── test.sh
├── include/
│   ├── astnode.h
│   ├── codegen.h
│   ├── context.h
│   ├── diagnostic.h
│   ├── parser.h
│   ├── tokenize.h
│   └── type.h
├── lib/
│   ├── codegen.cpp
│   ├── diagnostic.cpp
│   ├── parser.cpp
│   ├── tokenize.cpp
│   └── type.cpp
└── docs/
    └── chibicc-architecture-api-cn.md
```

## Notes for Readers

- Memory ownership is centralized in `ASTContext` using `std::unique_ptr`, while the parser and code generator work with raw non-owning pointers.
- The parser performs syntactic construction first, then `add_type()` decorates AST nodes with semantic type information used by later stages.
- Arrays are treated specially during code generation: array expressions decay to addresses in expression contexts, while array assignment is rejected.

## Roadmap

Reasonable next steps from the current codebase:

- Read source from files instead of a single CLI string
- Add function declarations and better semantic checks
- Implement string literals and array/global initialization beyond integer constants
- Support richer C types such as structs and unions
- Add a preprocessor or integrate with an external preprocessing step

## Acknowledgments

- [chibicc](https://github.com/rui314/chibicc) by Rui Ueyama
- [An Incremental Approach to Compiler Construction](http://scheme2006.cs.uchicago.edu/11-ghuloum.pdf)

## License

MIT. See [LICENSE](LICENSE).
