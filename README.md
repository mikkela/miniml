# Mini-ML Compiler (C++)

This is a toy compiler and runtime for a Mini-ML language, written in C++20.

The project demonstrates:
- A simple AST for Mini-ML expressions (lambda calculus + let, if, literals)
- Hindley–Milner–style type inference (simplified, no let-generalization yet)
- A custom IR (3-address code style) with a tiny IR builder
- A stub parser (ANTLR4 grammar provided, not yet wired)
- A GoogleTest-based test suite

## Getting Started

### Prerequisites
- CMake >= 3.22
- C++20 compiler (clang++, g++, or MSVC)
- [GoogleTest](https://github.com/google/googletest) is automatically fetched via CMake
- [ANTLR4](https://github.com/antlr/antlr4) runtime is optionally fetched when `-DENABLE_ANTLR=ON`
- LLVM is optional if you want a native backend (`-DENABLE_LLVM=ON`)

### Build
```bash
cmake -S . -B build -DENABLE_ANTLR=OFF -DENABLE_LLVM=OFF
cmake --build build
ctest --test-dir build --output-on-failure
```

### Run the demo
```bash
./build/minimlc
```

Expected output:
```
Type of program: (('0 -> '0))
=== IR ===
func main:
  movi #42 -> %t0
  call %t0
  ret %t0
```

### Run tests
```bash
ctest --test-dir build --output-on-failure
```

## Repository structure
```
src/
  ast/          AST node definitions
  types/        Type system (Type, Substitution, Unification, Inference)
  ir/           Intermediate Representation and builder
  parser/       Parser stubs (ANTLR grammar provided in lexer_parser/)
  backends/     (planned) LLVM, WASM, VAX backends
  repl/         (planned) REPL implementation
runtime/        (planned) GC and runtime system
tests/          GoogleTest-based unit tests
examples/       Example Mini-ML programs
docs/           Documentation
```

## Roadmap
- [x] Basic AST and type inference
- [x] Simple IR
- [ ] Let-generalization in type checker
- [ ] ADTs + pattern matching
- [ ] REPL
- [ ] LLVM backend
- [ ] WASM backend
- [ ] VAX backend (via SIMH + NetBSD)
- [ ] Custom GC runtime

## License
MIT (see LICENSE file if present).
