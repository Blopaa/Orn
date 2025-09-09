# Compiler

A **strong-typed, low-level programming language** with **light syntax** designed for performance and expressiveness. Features enhanced ternary operators, symbol-based loops, and a robust semantic **type checker** for compile-time safety.

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/Blopaa/Compiler)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Language](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_%28programming_language%29)
[![Testing](https://img.shields.io/badge/testing-Unity-green.svg)](https://github.com/ThrowTheSwitch/Unity)

---

## ✨ Core Features

* 🎯 **Strong Type System**: Compile-time type checking for `int`, `float`, `string`, `bool`
* 🔥 **Light Syntax**: Minimal keywords, enhanced ternary replaces if/else
* 🔄 **Symbol-Based Loops**: `@ x > 0 { x--; }` replaces `while`
* ⚡ **Low-Level Control**: Direct data manipulation without abstraction overhead
* 🎛️ **Enhanced Ternary**: `condition ? action` and `condition ? action : else`
* 🔧 **Full Expressions**: Arithmetic, logical, comparison with proper precedence
* 📦 **Block Statements**: Lightweight scoped code blocks
* 🧪 **Professional Testing**: 47+ Unity tests with full CI/CD integration
* 🔎 **Semantic Analysis**: Symbol table & type checker ensure correctness and prevent runtime surprises

---

## 🌟 Semantic Analysis & Type Checking

After parsing, the compiler performs **semantic validation** through a **type checker** (`src/semantic/typeChecker.c` + `src/semantic/symbolTable.c`).

### Features

* **Symbol Table Management**: Tracks variable declarations, types, scope depth, and initialization status.
* **Scope Handling**: Global scope plus nested block scopes with shadowing support.
* **Declaration Validation**: Prevents redeclaration in the same scope, enforces correct initializer types.
* **Assignment Validation**: Ensures left-hand side is a variable, type-compatible, and initialized.
* **Expression Validation**:

    * Numeric-only arithmetic operators.
    * Boolean-only logical operators.
    * Type promotion (`int → float`).
* **Error Handling**: Detailed error codes for type mismatches, uninitialized variables, and invalid expressions.

### Example

```c
int x;            // Declared but not initialized
x = 5;            // ✅ valid
float y = x;      // ✅ int → float allowed
bool b = x;       // ❌ type error: cannot assign int to bool
```
## 🌟 Enhanced Control Flow Syntax

### Enhanced Ternary (No if/else keywords)
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

### Symbol-Based Loops (No while/for keywords)
```c
// Traditional while:               // Enhanced while:
while (condition) {                 @ condition {
    statements;                         statements;
}                                   }

// Traditional for:                 // Enhanced for pattern:
for (int i = 0; i < 10; i++) {      int i = 0; @ i < 10; {
    process(i);                         process(i);
}                                       i++;
                                    }

// Inline example:
@ x > 0 { x--; print(x); }          // While x > 0, decrement and print
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

### Symbol-Based Loops
```c
// Basic while loops
@ x > 0 {
    x--;
    process(x);
}

@ !empty(queue) {
    item = dequeue();
    handle(item);
}

// For-like patterns (declaration + while)
int i = 0; @ i < 10; {
    print(i);
    i++;
}

float x = 1.0; @ x < 100.0; {
    result += x;
    x *= 2;                       // Geometric progression
}

// Complex conditions
@ balance > 0 && attempts < 5; {
    transaction = process_payment();
    balance -= transaction.amount;
    attempts++;
}

// Nested loops
int row = 0; @ row < height; {
    int col = 0; @ col < width; {
        matrix[row][col] = calculate(row, col);
        col++;
    }
    row++;
}
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

### Mixed Control Flow
```c
// Combining loops and conditionals
int count = 0; @ count < 1000; {
    count % 100 == 0 ? {
        print("Milestone: " + count);
        count > 500 ? break;      // Future: break statement
    };
    count++;
}

// Loop with conditional processing
bool found = false;
int i = 0; @ i < array_size && !found; {
    array[i] == target ? {
        found = true;
        index = i;
    };
    i++;
}
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
┌─────────────┐    ┌──────────────┐    ┌─────────────┐    ┌───────────────────┐
│   Source    │───▶│    Lexer     │───▶│   Parser    │───▶│ Semantic Analyzer │
│"@ x>0{x--;}"│    │ (Tokenizer)  │    │  (AST Gen)  │    │ (Type Checker)    │
└─────────────┘    └──────────────┘    └─────────────┘    └───────────────────┘
                          │                   │                       │
                          ▼                   ▼                       ▼
                   ┌──────────────┐    ┌─────────────┐        ┌──────────────┐
                   │   Tokens     │    │  AST Tree   │        │ Checked AST  │
                   │ [@,x,>,0,{,  │    └─────────────┘        └──────────────┘
                   │  x,--,;,}]   │
                   └──────────────┘

```

### Project Structure
```
Compiler/
├── src/
│   ├── main.c                 # Entry point and demo
│   ├── lexer/                 # Tokenization
│   ├── parser/                # AST generation
│   ├── semantic/              # Symbol table + type checker
│   └── errorHandling/         # Error reporting
├── test/                      # Unity tests
├── build/                     # Build directory
├── CMakeLists.txt             # Build configuration
└── README.md

```

## 🧪 Testing

Professional Unity testing framework with comprehensive coverage:

```bash
# Run all tests
./test_runner

# Expected output: XX Tests 0 Failures 0 Ignored - OK

# Memory leak detection (Linux/macOS)
valgrind --leak-check=full ./test_runner
```

**Test Categories:**
- ✅ Type declarations and assignments
- ✅ Arithmetic operations with precedence
- ✅ Enhanced ternary conditionals
- ✅ Block statements and scoping
- ✅ Symbol-based while loops
- ✅ Logical and comparison operators
- ✅ Increment/decrement operations
- ✅ Compound assignments
- ✅ Error handling and edge cases
- ✅ Complex mixed expressions

## 🔧 Language Grammar

```
Program           → Statement*
Statement         → VarDecl | Block | WhileLoop | ExprStatement
VarDecl           → Type IDENTIFIER ('=' Expression)? ';'
Block             → '{' Statement* '}'
WhileLoop         → '@' Expression Block
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

### Control Flow Patterns
| Pattern | Traditional | Enhanced | Example |
|---------|-------------|----------|---------|
| **If-only** | `if (cond) stmt` | `cond ? stmt` | `valid ? process();` |
| **If-else** | `if (cond) s1 else s2` | `cond ? s1 : s2` | `x > 0 ? pos() : neg();` |
| **While** | `while (cond) body` | `@ cond body` | `@ i < 10 { print(i); }` |
| **For-like** | `for(init;cond;inc) body` | `init; @ cond; body` | `int i=0; @ i<10; {code; i++;}` |

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