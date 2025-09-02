# Compiler

A foundational C-like programming language compiler written in C, demonstrating the core stages of compilation: **lexical analysis**, **parsing**, and **AST generation**. The compiler foundation is being built to support multiple backends (bytecode generation, VM interpretation, or transpilation).

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/Blopaa/Compiler)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Language](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Branch](https://img.shields.io/badge/branch-dev-orange.svg)](https://github.com/Blopaa/Compiler/tree/dev)

## ✨ Current Features

- 🔍 **Lexical Analysis**: Advanced tokenizer with support for strings, numbers, operators, and keywords
- 🌳 **AST Generation**: Clean Abstract Syntax Tree representation of parsed code
- 🎯 **Type System**: Strong type checking with comprehensive error messages for `int`, `float`, `string`, and `bool`
- ⚡ **Full Arithmetic Operations**: Support for `+`, `-`, `*`, `/`, `%` with proper precedence and associativity
- 🔢 **Increment/Decrement**: Prefix and postfix operators (`++`, `--`)
- 🔧 **Assignment Operations**: Basic assignments (`=`) and compound assignments (`+=`, `-=`, `*=`, `/=`)
- 💬 **Comment Support**: Single-line comments (`//`)
- 🛡️ **Error Handling**: Detailed error reporting system with specific error codes
- 🧪 **Testing Framework**: Comprehensive test suite with 95%+ coverage
- 📚 **Educational**: Clean, well-documented code perfect for learning compiler construction

## 🎯 Language Features

### Data Types
```c
int age = 25;           // Integer variables
float pi = 3.14159;     // Floating-point numbers  
string name = "Alice";  // String literals
bool active = true;     // Boolean values
```

### Arithmetic Operations
```c
int sum = a + b;        // Addition
int diff = x - y;       // Subtraction
int product = x * y;    // Multiplication
int quotient = x / y;   // Division
int remainder = x % y;  // Modulo
float result = 2.5 + 1.8;  // Mixed operations
int negative = -42;     // Negative numbers
```

### Increment/Decrement Operations
```c
int counter = 5;
int pre_inc = ++counter;   // Pre-increment (counter becomes 6, pre_inc = 6)
int post_inc = counter++;  // Post-increment (post_inc = 6, counter becomes 7)
int pre_dec = --counter;   // Pre-decrement (counter becomes 6, pre_dec = 6)
int post_dec = counter--;  // Post-decrement (post_dec = 6, counter becomes 5)
```

### Assignment Operations
```c
int x = 10;            // Basic assignment
x += 5;                // Compound addition (x = x + 5)
y -= 3;                // Compound subtraction (y = y - 3)
z *= 2;                // Compound multiplication (z = z * 2)
w /= 4;                // Compound division (w = w / 4)
```

### Expression Precedence
```c
int result = 2 + 3 * 4;      // Result: 14 (multiplication first)
int calc = (10 - 3) * 2;     // Result: 14 (parentheses first)
int complex = --x + y++ - ++z; // Mixed increment/decrement operations
```

### Comments
```c
// This is a single-line comment
int x = 5; // Inline comment
```

### Type Safety
```c
int x = "hello";        // ❌ Error: Cannot assign string to int
string name = 123;      // ❌ Error: Cannot assign int to string
float bad = 3.14.15;    // ❌ Error: Invalid float format
bool flag = "yes";      // ❌ Error: Cannot assign string to bool
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
Input: int test = --2 + 3;

1. SPLITTING:
split 0: 'int'
split 1: 'test'  
split 2: '='
split 3: '--'
split 4: '2'
split 5: '+'
split 6: '3'
split 7: ';'
Total tokens: 8

2. TOKENIZATION:
Token 0: 'int', type: 2
Token 1: 'test', type: 1  
Token 2: '=', type: 0
Token 3: '--', type: 17
Token 4: '2', type: 1
Token 5: '+', type: 9
Token 6: '3', type: 1
Token 7: ';', type: 5
Total tokens processed: 8

3. AST GENERATION:
AST:
└── INT_VAR_DEF: test
    └── ADD_OP
        ├── PRE_DECREMENT
        │   └── INT_LIT: 2
        └── INT_LIT: 3

Compilation successful: No errors or warnings.
```

## 🗂️ Architecture

```
┌─────────────┐    ┌──────────────┐    ┌─────────────┐
│   Input     │───▶│    Lexer     │───▶│   Parser    │
│ "int x=--5;"│    │ (Tokenizer)  │    │ (AST Gen)   │
└─────────────┘    └──────────────┘    └─────────────┘
                           │                    │
                           ▼                    ▼
                   ┌──────────────┐    ┌─────────────┐
                   │   Tokens     │    │     AST     │
                   │[int,x,=,--,5]│    │    Tree     │
                   └──────────────┘    └─────────────┘
```

### Core Components

| Component | Description | File |
|-----------|-------------|------|
| **Lexer** | Tokenizes input, handles strings, numbers, operators, comments | `lexer/lexer.c` |
| **Parser** | Generates AST from tokens, type checking, precedence handling | `parser/parser.c` |
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
✅ Basic int declaration
✅ Basic string declaration
✅ Basic float declaration
✅ Basic bool declaration
✅ Simple addition
✅ Complex precedence: 2 + 3 * 4
✅ Prefix increment assignment
✅ Postfix decrement assignment
✅ Basic modulo: 10 % 3
✅ Compound assignment with expression

=== TESTING ERROR CASES ===
✅ String to int error
✅ Invalid float (multiple decimals)
✅ Bool to string error

=== TEST SUMMARY ===
Tests run: 65
Tests passed: 65
Tests failed: 0
🎉 All tests passed!
Success rate: 100.0%
```

## 📖 Examples

<details>
<summary><b>Example 1: Variable Declarations with All Types</b></summary>

**Input:**
```c
int age = 30;
string name = "Developer";
float height = 5.9;
bool active = true;
```

**AST Output:**
```
AST:
├── INT_VAR_DEF: age
│   └── INT_LIT: 30
├── STRING_VAR_DEF: name  
│   └── STRING_LIT: "Developer"
├── FLOAT_VAR_DEF: height
│   └── FLOAT_LIT: 5.9
└── BOOL_VAR_DEF: active
    └── BOOL_LIT: true
```
</details>

<details>
<summary><b>Example 2: Complex Arithmetic Expressions</b></summary>

**Input:**
```c
int result = 2 + 3 * 4 - 1;
int remainder = value % 10;
int complex = --counter + index++;
```

**AST Output:**
```
AST:
├── INT_VAR_DEF: result
│   └── SUB_OP
│       ├── ADD_OP
│       │   ├── INT_LIT: 2
│       │   └── MUL_OP
│       │       ├── INT_LIT: 3
│       │       └── INT_LIT: 4
│       └── INT_LIT: 1
├── INT_VAR_DEF: remainder
│   └── MOD_OP
│       ├── VARIABLE: value
│       └── INT_LIT: 10
└── INT_VAR_DEF: complex
    └── ADD_OP
        ├── PRE_DECREMENT
        │   └── VARIABLE: counter
        └── POST_INCREMENT
            └── VARIABLE: index
```
</details>

<details>
<summary><b>Example 3: Compound Assignments</b></summary>

**Input:**
```c
total += value * 2;
counter -= 1;
balance *= 1.05;
score /= attempts;
```

**AST Output:**
```
AST:
├── COMPOUND_ADD_ASSIGN: total
│   └── MUL_OP
│       ├── VARIABLE: value
│       └── INT_LIT: 2
├── COMPOUND_SUB_ASSIGN: counter
│   └── INT_LIT: 1
├── COMPOUND_MUL_ASSIGN: balance
│   └── FLOAT_LIT: 1.05
└── COMPOUND_DIV_ASSIGN: score
    └── VARIABLE: attempts
```
</details>

<details>
<summary><b>Example 4: Comments and Mixed Code</b></summary>

**Input:**
```c
// Initialize variables
int x = 5;
y = x + 1; // Assign to existing variable
// Calculate result
result = x * y + 10;
```

**AST Output:**
```
AST:
├── INT_VAR_DEF: x
│   └── INT_LIT: 5
├── ASSIGNMENT: y
│   └── ADD_OP
│       ├── VARIABLE: x
│       └── INT_LIT: 1
└── ASSIGNMENT: result
    └── ADD_OP
        ├── MUL_OP
        │   ├── VARIABLE: x
        │   └── VARIABLE: y
        └── INT_LIT: 10
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
│   │   └── lexer.h            # Lexer interface and token definitions
│   ├── parser/  
│   │   ├── parser.c           # AST generation and type checking
│   │   └── parser.h           # Parser interface and AST node types
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
Program           → Statement*
Statement         → VarDecl | Assignment | CompoundAssign
VarDecl           → Type IDENTIFIER '=' Expression ';'
Assignment        → IDENTIFIER '=' Expression ';'
CompoundAssign    → IDENTIFIER CompoundOp Expression ';'
Expression        → AddSubExpr
AddSubExpr        → MulDivModExpr (('+' | '-') MulDivModExpr)*
MulDivModExpr     → UnaryExpr (('*' | '/' | '%') UnaryExpr)*
UnaryExpr         → ('++' | '--') PrimaryExpr | PrimaryExpr ('++' | '--')?
PrimaryExpr       → NUMBER | STRING | BOOLEAN | IDENTIFIER | '-' NUMBER
Type              → 'int' | 'float' | 'string' | 'bool'
CompoundOp        → '+=' | '-=' | '*=' | '/='
BOOLEAN           → 'true' | 'false'
```

### Adding New Features

1. **New Token Types**: Update `TokenType` enum in `lexer/lexer.h`
2. **New AST Nodes**: Add to `NodeTypes` in `parser/parser.h`
3. **Error Types**: Extend `ErrorCode` in `errorHandling/errorHandling.h`
4. **Parser Logic**: Update parsing functions in `parser/parser.c`
5. **Tests**: Add test cases in `testing/testing.c`

### Operator Precedence (Highest to Lowest)
1. **Postfix/Prefix**: `++`, `--`
2. **Unary**: `-` (negative)
3. **Multiplicative**: `*`, `/`, `%`
4. **Additive**: `+`, `-`
5. **Assignment**: `=`, `+=`, `-=`, `*=`, `/=`

## 🚧 Roadmap

**Phase 1: Core Language (✅ COMPLETED)**
- [x] Lexical analysis and tokenization
- [x] AST generation and parsing
- [x] Type system (int, float, string, bool)
- [x] Full arithmetic expressions (+, -, *, /, %)
- [x] Increment/decrement operators (++, --)
- [x] Compound assignments (+=, -=, *=, /=)
- [x] Comment support (//)
- [x] Operator precedence and associativity
- [x] Error handling and reporting
- [x] Comprehensive testing framework

**Phase 2: Control Flow**
- [ ] **Conditional Statements**: `if/else` statements
- [ ] **Loops**: `while`, `for` statements
- [ ] **Comparison Operators**: `==`, `!=`, `<`, `>`, `<=`, `>=`
- [ ] **Logical Operators**: `&&`, `||`, `!`
- [ ] **Block Statements**: `{ }` scope handling

**Phase 3: Functions and Scope**
- [ ] **Function Declarations**: `func name(params) { }`
- [ ] **Function Calls**: `name(args)`
- [ ] **Return Statements**: `return value;`
- [ ] **Local Scope**: Variable scoping rules
- [ ] **Parameter Passing**: By value, by reference

**Phase 4: Backend Selection**
- [ ] **Bytecode Compiler**: Generate custom bytecode
- [ ] **Virtual Machine**: Build interpreter with VM
- [ ] **Transpiler**: Generate C, JavaScript, or other languages
- [ ] **Code Generation**: Direct assembly or LLVM IR

**Phase 5: Advanced Features**
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
- ✅ Add test cases for both success and error scenarios

## 🗝️ Architecture Notes

This project is designed as a **compiler foundation** that can support multiple backends:

- **Compiler**: Generate bytecode or machine code
- **Interpreter**: Build a virtual machine to execute ASTs
- **Transpiler**: Generate code in another high-level language

The current implementation focuses on the **frontend** (lexer → parser → AST), providing a solid foundation for any backend choice. The clean separation of concerns makes it easy to extend in any direction.

### Key Design Decisions

1. **Recursive Descent Parser**: Easy to understand and extend
2. **AST-Based**: Clean representation for further processing
3. **Strong Type Checking**: Catch errors early in compilation
4. **Comprehensive Testing**: Ensures reliability and catch regressions
5. **Modular Design**: Each phase is independent and testable

<div align="center">

**⭐ If you found this helpful, please consider giving it a star! ⭐**

[Report Bug](https://github.com/Blopaa/Compiler/issues) • [Request Feature](https://github.com/Blopaa/Compiler/issues) • [Ask Question](https://github.com/Blopaa/Compiler/discussions)

</div>