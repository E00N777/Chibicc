# Chibicc — A Toy C Compiler Rewritten in C++

A from-scratch C compiler targeting x86-64 Linux, rewritten in modern C++ (C++20).
Based on [Rui Ueyama's chibicc](https://github.com/rui314/chibicc), this project reimplements the compiler with C++ idioms — classes, RAII-based memory management, `std::unique_ptr`, `std::string_view`, and clean separation of concerns.

This is primarily an educational project: a learning journal for understanding how compilers work from tokenization to code generation.

## Features

- **Tokenizer** — Lexes integer literals, identifiers, keywords, and multi-character operators (`==`, `!=`, `>=`, `<=`)
- **Recursive-descent parser** — Precedence-climbing expression parser with support for:
  - Arithmetic: `+`, `-`, `*`, `/`, unary `-`
  - Comparison: `==`, `!=`, `<`, `<=`, `>`, `>=`
  - Assignment: `=`
  - Pointer arithmetic with automatic scaling (ptr + int, ptr - ptr)
- **Type system** — `int` and pointer-to-T types, with bottom-up type propagation across the AST
- **Declarations** — `int` variables with optional initializers, pointer declarators (`int *p`, `int **pp`), and comma-separated declarations (`int x=3, y=5;`)
- **Control flow** — `if`/`else`, `while`, `for`, `return`, nested block scoping `{}`
- **Function calls** — Supports calling external C functions with up to 6 arguments (x86-64 System V ABI)
- **Address-of / Dereference** — `&` and `*` operators for pointer manipulation
- **x86-64 code generation** — Emits AT&T syntax assembly to stdout, linkable with GCC
- **Centralized memory management** — All AST nodes, tokens, types, and objects are owned by a single `ASTContext` via `std::unique_ptr`, providing automatic cleanup with zero manual `delete`

## Project Structure

```
Chibicc/
├── Main.cpp                 # Entry point: tokenize → parse → codegen
├── CMakeLists.txt           # Build configuration (C++20, static library + executable)
├── test.sh                  # Integration test suite (shell-based, 100+ assertions)
│
├── include/
│   ├── tokenize.h           # Token class and Tokenize() interface
│   ├── astnode.h            # AST Node, Obj (local variable), Function definitions
│   ├── parser.h             # Recursive-descent Parser class
│   ├── type.h               # Type system: TypeKind, Type class, type utilities
│   ├── context.h            # ASTContext: centralized arena-style memory owner
│   ├── codegen.h            # x86-64 CodeGen class
│   └── diagnostic.h         # Error reporting utilities
│
├── lib/
│   ├── tokenize.cpp         # Lexer implementation
│   ├── parser.cpp           # Parser implementation (expression, statement, declaration)
│   ├── type.cpp             # Type propagation (add_type) and type utilities
│   ├── codegen.cpp          # x86-64 assembly emission
│   └── diagnostic.cpp       # Error formatting and fatal exit
│

```

## Architecture

```
Source string (argv[1])
        │
        ▼
   ┌──────────┐
   │ Tokenizer │──→  Token linked list
   └──────────┘
        │
        ▼
   ┌──────────┐
   │  Parser   │──→  AST (Node tree) + Function + Obj list
   └──────────┘
        │
        ▼
   ┌──────────┐
   │  CodeGen  │──→  x86-64 assembly (stdout)
   └──────────┘
```

All heap-allocated objects (Token, Node, Obj, Function, Type) are created through `ASTContext` factory methods and stored in `std::vector<std::unique_ptr<T>>`. When `ASTContext` goes out of scope at the end of `main()`, everything is automatically freed — no manual `delete` required.

## Building

Requires a C++20 compiler (GCC 10+ or Clang 12+) and CMake 3.10+.

```bash
git clone https://github.com/E00N777/Chibicc.git  && cd Chibicc
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

## Running

The compiler reads a single C function body from the command line and emits x86-64 assembly to stdout:

```bash
./build/Chibicc '{ int x=3; int y=5; return x+y; }' > tmp.s
gcc -static -o tmp tmp.s
./tmp
echo $?   # prints 8
```

## Testing

The test suite (`test.sh`) compiles helper C functions with GCC, then runs 100+ assertions covering arithmetic, variables, pointers, control flow, and function calls:

```bash
cd build
ctest
# or run directly:
../test.sh ./Chibicc
```

## Supported C Subset

| Category          | Examples                                       |
|-------------------|-------------------------------------------------|
| Integer literals  | `0`, `42`, `-10`                                |
| Variables         | `int a=3;` `int *p=&a;` `int x, y;`            |
| Arithmetic        | `a+b`, `a*b/c`, `-x`                            |
| Comparison        | `a==b`, `a!=b`, `a<b`, `a<=b`, `a>b`, `a>=b`   |
| Pointers          | `&x`, `*p`, `**pp`, pointer arithmetic           |
| Control flow      | `if/else`, `while`, `for`, `return`, `{}`       |
| Function calls    | `add(3, 5)`, up to 6 args (System V ABI)        |

## Roadmap

Planned features (following the original chibicc progression):

- [ ] Multiple function definitions
- [ ] `char` type and string literals
- [ ] Arrays and `sizeof`
- [ ] Global variables
- [ ] Structs and unions
- [ ] Preprocessor

## Acknowledgments

- [chibicc](https://github.com/rui314/chibicc) by Rui Ueyama — the original C-language implementation this project is based on
- [An Incremental Approach to Compiler Construction](http://scheme2006.cs.uchicago.edu/11-ghuloum.pdf) — the pedagogical philosophy behind chibicc

## License

MIT — see [LICENSE](LICENSE) for details.
