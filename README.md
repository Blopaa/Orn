# Compiler

A foundational C-like programming language compiler written in C, demonstrating the core stages of compilation: **lexical analysis**, **parsing**, and **AST generation**. The compiler foundation is being built to support multiple backends (bytecode generation, VM interpretation, or transpilation).

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/Blopaa/Compiler)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Language](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Branch](https://img.shields.io/badge/branch-dev-orange.svg)](https://github.com/Blopaa/Compiler/tree/dev)

## ✨ Current Features

- 🔍 **Lexical Analysis**: Advanced tokenizer with support for strings, numbers, operators, and keywords
- 🌳 **AST Generation**: Clean Abstract Syntax Tree representation of parsed code
- 🎯 **Type System**: Strong type checking with comprehensive error messages
- ⚡ **Expression Parsing**: Support for arithmetic expressions (`+`, `-`) and negative numbers
- 🔧 **Assignment Operations**: Basic assignments (`=`) and compound assignments (`+=`, `-=`, `*=`, `/=`)
- 🛡️ **Error Handling**: Detailed error reporting system with specific error codes
- 🧪 **Testing Framework**: Comprehensive test suite with 95%+ coverage
- 📚 **Educational**: Clean, well-documented code perfect for learning compiler construction

## 🎯Language Features

### Data Types
```
int age = 25;           // Integer variables
float pi = 3.14159;     // Floating-point numbers  
string name = "Alice";  // String literals
```

### Arithmetic Operations
```
int sum = a + b;        // Addition
int diff = x - y;       // Subtraction
float result = 2.5 + 1.8;  // Mixed operations
int negative = -42;     // Negative numbers
```

### Assignment Operations
```
int x = 10;            // Basic assignment
x += 5;                // Compound addition
y -= 3;                // Compound subtraction
z *= 2;                // Compound multiplication
w /= 4;                // Compound division
```

### Type Safety
```
int x = "hello";        // ❌ Error: Cannot assign string to int
string name = 123;      // ❌ Error: Cannot assign int to string
float bad = 3.14.15;    // ❌ Error: Invalid float format
```

## 🚀 Quick Start

### Prerequisites
- GCC compiler or Clang
- CMake (3.10 or higher)

### Installation

1. **Clone the repository**
   ```bash
   git clone https://github.com/Blopaa/Compiler.git
   cd Compiler
   
   # Switch to development branch
   git checkout dev
   ```

2. **Build the compiler**
   ```bash
   mkdir build && cd build
   cmake ..
   make
   
   # Or use CMake directly:
   cmake --build .
   ```

3. **Run tests**
   ```bash
   ./compiler --test
   ```

### Basic Usage

**Interactive Mode:**
```bash
./compiler
```

The compiler processes the default example and shows the compilation pipeline:
```
Input: int temperature = -15;

1. SPLITTING:
split 0: 'int'
split 1: 'temperature'  
split 2: '='
split 3: '-15'
split 4: ';'
Total tokens: 5

2. TOKENIZATION:
Token 0: 'int', type: 2
Token 1: 'temperature', type: 1  
Token 2: '=', type: 0
Token 3: '-15', type: 1
Token 4: ';', type: 5
Total tokens processed: 5

3. AST GENERATION:
AST:
└── INT_VAR_DEF: temperature
    └── INT_LIT: -15

Compilation successful: No errors or warnings.
```

## 🗂️ Architecture

```
┌─────────────┐    ┌──────────────┐    ┌─────────────┐
│   Input     │───▶│    Lexer     │───▶│   Parser    │
│ "int x=-5;" │    │ (Tokenizer)  │    │ (AST Gen)   │
└─────────────┘    └──────────────┘    └─────────────┘
                           │                    │
                           ▼                    ▼
                   ┌──────────────┐    ┌─────────────┐
                   │   Tokens     │    │     AST     │
                   │[int,x,=,-5,;]│    │    Tree     │
                   └──────────────┘    └─────────────┘
```

### Core Components

| Component | Description | File |
|-----------|-------------|------|
| **Lexer** | Tokenizes input, handles strings, numbers, operators | `lexer/lexer.c` |
| **Parser** | Generates AST from tokens, type checking | `parser/parser.c` |
| **Error Handler** | Comprehensive error reporting with specific codes | `errorHandling/errorHandling.c` |
| **Testing** | Automated test suite with edge cases | `testing/testing.c` |

## 🧪 Testing

Run the comprehensive test suite:

```bash
./compiler --test
```

**Example output:**
```
=== TESTING BASIC CASES ===
Testing: Basic int declaration
Input: int x = 5;
✅ AST should be generated
✅ Should have no errors

Testing: Basic negative int
Input: int x = -5;
✅ AST should be generated
✅ Should have no errors

=== TESTING ERROR CASES ===
Testing: String to int error
Input: int x = "hello";
✅ Should have errors

=== TEST SUMMARY ===
Tests run: 45
Tests passed: 45
Tests failed: 0
🎉 All tests passed!
Success rate: 100.0%
```

## 📖 Examples

<details>
<summary><b>Example 1: Variable Declarations</b></summary>

**Input:**
```
int age = 30;
string name = "Developer";
float height = 5.9;
```

**AST Output:**
```
AST:
├── INT_VAR_DEF: age
│   └── INT_LIT: 30
├── STRING_VAR_DEF: name  
│   └── STRING_LIT: "Developer"
└── FLOAT_VAR_DEF: height
    └── FLOAT_LIT: 5.9
```
</details>

<details>
<summary><b>Example 2: Arithmetic Expressions</b></summary>

**Input:**
```
int sum = a + b;
int negative = -42;
```

**AST Output:**
```
AST:
├── INT_VAR_DEF: sum
│   └── ADD_OP
│       ├── VARIABLE: a
│       └── VARIABLE: b
└── INT_VAR_DEF: negative
    └── INT_LIT: -42
```
</details>

<details>
<summary><b>Example 3: Compound Assignments</b></summary>

**Input:**
```
total += value;
counter -= 1;
```

**AST Output:**
```
AST:
├── COMPOUND_ADD_ASSIGN: total
│   └── VARIABLE: value
└── COMPOUND_SUB_ASSIGN: counter
    └── INT_LIT: 1
```
</details>

## 🛠️ Development

### Project Structure
```
Compiler/
├── src/
│   ├── main.c                 # Entry point and demo
│   ├── errorHandling/
│   │   ├── errorHandling.c    # Error management system
│   │   └── errorHandling.h    # Error definitions and codes
│   ├── lexer/
│   │   ├── lexer.c            # Tokenization logic
│   │   └── lexer.h            # Lexer interface
│   ├── parser/  
│   │   ├── parser.c           # AST generation and type checking
│   │   └── parser.h           # Parser interface
│   └── testing/
│       ├── testing.c          # Comprehensive test framework
│       └── testing.h          # Test interface
├── CMakeLists.txt            # CMake build system
├── CONTRIBUTING.md           # Contribution guidelines
├── LICENSE.md               # Project license
├── README.md               # This file
└── .gitignore              # Git ignore rules
```

### Current Language Grammar

```
Program      → Statement*
Statement    → VarDecl | Assignment | CompoundAssign
VarDecl      → Type IDENTIFIER '=' Expression ';'
Assignment   → IDENTIFIER '=' Expression ';'
CompoundAssign → IDENTIFIER CompoundOp Expression ';'
Expression   → Term | Term ('+' | '-') Term
Term         → NUMBER | STRING | IDENTIFIER | '-' NUMBER
Type         → 'int' | 'float' | 'string'
CompoundOp   → '+=' | '-=' | '*=' | '/='
```

### Adding New Features

1. **New Token Types**: Update `TokenType` enum in `lexer/lexer.h`
2. **New AST Nodes**: Add to `NodeTypes` in `parser/parser.h`
3. **Error Types**: Extend `ErrorCode` in `errorHandling/errorHandling.h`
4. **Tests**: Add cases in `testing/testing.c`

## 🚧 Roadmap

**Phase 1: Core Language (Current)**
- [x] Lexical analysis and tokenization
- [x] AST generation and parsing
- [x] Basic type system (int, float, string)
- [x] Arithmetic expressions (+, -)
- [x] Compound assignments (+=, -=, *=, /=)
- [x] Error handling and reporting
- [x] Comprehensive testing framework

**Phase 2: Language Extensions**
- [ ] **Control Flow**: `if/else`, `while`, `for` statements
- [ ] **More Operations**: Multiplication, division, modulo
- [ ] **Comparison Operators**: `==`, `!=`, `<`, `>`, `<=`, `>=`
- [ ] **Boolean Type**: `bool` with `true/false` literals
- [ ] **Functions**: Function declarations and calls

**Phase 3: Backend Selection**
- [ ] **Bytecode Compiler**: Generate custom bytecode
- [ ] **Virtual Machine**: Build interpreter with VM
- [ ] **Transpiler**: Generate C, JavaScript, or other languages
- [ ] **Code Generation**: Direct assembly or LLVM IR

**Phase 4: Advanced Features**
- [ ] **Arrays**: Static and dynamic arrays
- [ ] **Structs**: User-defined types
- [ ] **Standard Library**: Built-in functions and I/O
- [ ] **Memory Management**: Automatic or manual
- [ ] **Optimization**: Basic compiler optimizations

## 🤝 Contributing

We welcome contributions! Here's how to get started:

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Add tests** for your changes
4. **Ensure** all tests pass (`./compiler --test`)
5. **Commit** your changes (`git commit -m 'Add amazing feature'`)
6. **Push** to the branch (`git push origin feature/amazing-feature`)
7. **Open** a Pull Request

### Contribution Guidelines
- ✅ Write tests for new features
- ✅ Follow existing code style
- ✅ Update documentation
- ✅ Keep commits atomic and well-described
- ✅ Ensure error handling for edge cases

## 🏗️ Architecture Notes

This project is designed as a **compiler foundation** that can support multiple backends:

- **Compiler**: Generate bytecode or machine code
- **Interpreter**: Build a virtual machine to execute ASTs
- **Transpiler**: Generate code in another high-level language

The current implementation focuses on the **frontend** (lexer → parser → AST), providing a solid foundation for any backend choice. The clean separation of concerns makes it easy to extend in any direction.

<div align="center">

**⭐ If you found this helpful, please consider giving it a star! ⭐**

[Report Bug](https://github.com/Blopaa/Compiler/issues) • [Request Feature](https://github.com/Blopaa/Compiler/issues) • [Ask Question](https://github.com/Blopaa/Compiler/discussions)

</div>
