# Compiler

A **strong-typed, low-level programming language** with **light syntax** designed for performance and expressiveness. Features enhanced ternary operators that replace traditional if/else statements for cleaner, more concise code.

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/Blopaa/Compiler)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Language](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Testing](https://img.shields.io/badge/testing-Unity-green.svg)](https://github.com/ThrowTheSwitch/Unity)

## ✨ Core Features

- 🎯 **Strong Type System**: Compile-time type checking for `int`, `float`, `string`, `bool`
- 🔥 **Light Syntax**: Minimal keywords, enhanced ternary replaces if/else
- ⚡ **Low-Level Control**: Direct data manipulation without abstraction overhead
- 🎛️ **Enhanced Ternary**: `condition ? action` and `condition ? action : else`
- 🔧 **Full Expressions**: Arithmetic, logical, comparison with proper precedence
- 📦 **Block Statements**: Lightweight scoped code blocks
- 🧪 **Professional Testing**: 47 Unity tests with full CI/CD integration

## 🌟 Enhanced Ternary Syntax

**No if/else keywords** - Clean conditional syntax:

```c
// Traditional if/else:             // Enhanced ternary:
if (condition) {                    condition ? {
    action1();                          action1();
    action2();                          action2();
}                                   };

if (x > 0) {                        x > 0 ? {
    result = x;                         result = x;
} else {                            } : {
    result = 0;                         result = 0;
}                                   };
```

## 🎯 Language Examples

### Type System & Declarations
```c
int count = 42;                   // Strong typing required
float rate = 3.14159;             // Precise numeric types
string name = "Compiler";         // String literals
bool active = true;               // Boolean values
```

### Expressions & Operations
```c
// Arithmetic with precedence
int result = 2 + 3 * 4;           // Result: 14 (multiply first)
float calc = value / 2.0;         // Type-aware operations

// Increment/Decrement
int pre = ++counter;              // Pre-increment
int post = value--;               // Post-decrement

// Compound assignments
total += x * 2;                   // Equivalent to: total = total + (x * 2)
balance *= 1.05;                  // Compound operations

// Logical and comparison
bool valid = x > 0 && y <= 100;   // Logical operators
bool equal = a == b;              // Comparison operators
bool check = !condition;          // Logical NOT
```

### Enhanced Ternary Conditionals
```c
// If-only statements
authenticated ? access_granted();
valid_input ? process_data();

// Ternary assignment
int max = a > b ? a : b;
string status = online ? "Connected" : "Offline";

// Block conditionals
score >= 90 ? {
    grade = "A";
    bonus_points += 10;
} : {
    grade = "B";
    bonus_points += 5;
};

// Compound operations in conditionals
balance > 0 ? account += interest : account -= fee;
```

### Block Statements
```c
{
    int local_var = 10;           // Block-scoped variables
    float temp = local_var * 2.5;
    result = temp;
}
// local_var and temp out of scope here
```

## 🚀 Quick Start

### Prerequisites
- GCC or Clang compiler
- CMake (3.10+)
- Git

### Build & Run
```bash
# Clone and setup
git clone https://github.com/Blopaa/Compiler.git
cd Compiler
git checkout dev

# Add Unity testing framework
git submodule add https://github.com/ThrowTheSwitch/Unity.git test/unity
git submodule update --init --recursive

# Build
mkdir build && cd build
cmake ..
make

# Run compiler demo
./compiler

# Run test suite
./test_runner
```

## 🗂️ Architecture

```
┌─────────────┐    ┌──────────────┐    ┌─────────────┐
│   Source    │───▶│    Lexer     │───▶│   Parser    │
│ "x > 0 ? y" │    │ (Tokenizer)  │    │  (AST Gen)  │
└─────────────┘    └──────────────┘    └─────────────┘
                          │                   │
                          ▼                   ▼
                   ┌──────────────┐    ┌─────────────┐
                   │   Tokens     │    │  AST Tree   │
                   │ [x,>,0,?,y]  │    └─────────────┘          
                   └──────────────┘    
```

### Project Structure
```
Compiler/
├── src/
│   ├── main.c                 # Entry point and demo
│   ├── lexer/                 # Tokenization (lexer.c/h)
│   ├── parser/                # AST generation (parser.c/h, parserHelpers.c)
│   └── errorHandling/         # Error management (errorHandling.c/h)
├── test/
│   ├── unity/                 # Unity testing framework (submodule)
│   └── test_main.c           # Complete test suite (47 tests)
├── build/                    # Build directory
├── CMakeLists.txt           # Build configuration
└── README.md               # This file
```

## 🧪 Testing

Professional Unity testing framework with comprehensive coverage:

```bash
# Run all 47 tests
./test_runner

# Expected output: 47 Tests 0 Failures 0 Ignored - OK

# Memory leak detection (Linux/macOS)
valgrind --leak-check=full ./test_runner
```

**Test Categories:**
- ✅ Type declarations and assignments
- ✅ Arithmetic operations with precedence
- ✅ Enhanced ternary conditionals
- ✅ Block statements and scoping
- ✅ Logical and comparison operators
- ✅ Increment/decrement operations
- ✅ Compound assignments
- ✅ Error handling and edge cases
- ✅ Complex mixed expressions

## 🔧 Language Grammar

```
Program           → Statement*
Statement         → VarDecl | Block | ExprStatement
VarDecl           → Type IDENTIFIER ('=' Expression)? ';'
Block             → '{' Statement* '}'
Expression        → EnhancedTernary
EnhancedTernary   → LogicalOr ('?' (Expression | Block) (':' (Expression | Block))?)?
LogicalOr         → LogicalAnd ('||' LogicalAnd)*
LogicalAnd        → Equality ('&&' Equality)*
Equality          → Comparison (('=='|'!=') Comparison)*
Comparison        → Term (('<'|'>'|'<='|'>=') Term)*
Term              → Factor (('+'|'-') Factor)*
Factor            → Unary (('*'|'/'|'%') Unary)*
Unary             → ('!'|'++'|'--'|'-') Unary | Primary ('++'|'--')?
Primary           → NUMBER | STRING | BOOLEAN | IDENTIFIER | '(' Expression ')'
```

### Operator Precedence (High to Low)
1. **Postfix/Prefix**: `++`, `--`, `!`, unary `-`
2. **Multiplicative**: `*`, `/`, `%`
3. **Additive**: `+`, `-`
4. **Comparison**: `<`, `>`, `<=`, `>=`
5. **Equality**: `==`, `!=`
6. **Logical AND**: `&&`
7. **Logical OR**: `||`
8. **Enhanced Ternary**: `?:`
9. **Assignment**: `=`, `+=`, `-=`, `*=`, `/=`

## 🤝 Contributing

We welcome contributions! To get started:

1. **Fork** the repository and clone with `--recursive`
2. **Create** a feature branch (`git checkout -b feature/new-feature`)
3. **Add Unity tests** for your changes in `test/test_main.c`
4. **Ensure** all tests pass (`./test_runner`)
5. **Follow** existing code style and documentation standards
6. **Submit** a Pull Request

### Adding Tests
```c
void test_your_feature(void) {
    Input res = splitter("your test code;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);
    
    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());
    
    freeTokenList(tokens);
    freeAST(ast);
}

// Add to main(): RUN_TEST(test_your_feature);
```

See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.

---

<div align="center">

**⭐ If you find this project interesting, please give it a star! ⭐**

[Report Bug](https://github.com/Blopaa/Compiler/issues) • [Request Feature](https://github.com/Blopaa/Compiler/issues) • [Documentation](CONTRIBUTING.md)

*Strong-typed, low-level programming language with light syntax*

</div>