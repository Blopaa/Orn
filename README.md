# Compiler

A foundational C-like programming language compiler written in C, demonstrating the core stages of compilation: **lexical analysis**, **parsing**, and **AST generation**. The compiler foundation is being built to support multiple backends (bytecode generation, VM interpretation, or transpilation).

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/Blopaa/Compiler)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Language](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![branch](https://img.shields.io/badge/branch-dev-orange.svg)](https://github.com/Blopaa/Compiler/tree/main)
[![Testing](https://img.shields.io/badge/testing-Unity-green.svg)](https://github.com/ThrowTheSwitch/Unity)

## ✨ Current Features

- 🔍 **Lexical Analysis**: Advanced tokenizer with support for strings, numbers, operators, and keywords
- 🌳 **AST Generation**: Clean Abstract Syntax Tree representation of parsed code
- 🎯 **Type System**: Strong type checking with comprehensive error messages for `int`, `float`, `string`, and `bool`
- ⚡ **Full Arithmetic Operations**: Support for `+`, `-`, `*`, `/`, `%` with proper precedence and associativity
- 🔢 **Increment/Decrement**: Prefix and postfix operators (`++`, `--`)
- 🔧 **Assignment Operations**: Basic assignments (`=`) and compound assignments (`+=`, `-=`, `*=`, `/=`)
- 💬 **Comment Support**: Single-line comments (`//`)
- 🛡️ **Error Handling**: Detailed error reporting system with specific error codes
- 🧪 **Professional Testing**: Comprehensive test suite using Unity framework with CI/CD integration
- 📚 **Educational**: Clean, well-documented code perfect for learning compiler construction

## 🎯 Language Features

### Data Types
```c
int age = 25;           // Integer variables
float pi = 3.14159;     // Floating-point numbers  
string name = "Alice";  // String literals
bool active = true;     // Boolean values
bool inactive = false;  // Boolean false value
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
float neg_float = -3.14; // Negative floats
```

### Comparison Operations
```c
bool equal = a == b;        // Equality comparison
bool not_equal = x != y;    // Inequality comparison
bool less = value < limit;  // Less than
bool greater = score > 90;  // Greater than
bool less_eq = age <= 65;   // Less than or equal
bool greater_eq = temp >= 0; // Greater than or equal
```

### Logical Operations
```c
bool and_result = x > 0 && y < 10;     // Logical AND
bool or_result = flag1 || flag2;       // Logical OR
bool not_result = !condition;          // Logical NOT
bool complex = (a > b) && (c <= d);    // Complex logical expressions
bool chain = x == y || z != w;         // Mixed logical and comparison
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
int result = 2 + 3 * 4;           // Result: 14 (multiplication first)
int calc = (10 - 3) * 2;          // Result: 14 (parentheses first)
int complex = --x + y++ - ++z;    // Mixed increment/decrement operations
bool comparison = a + b >= c * 2;  // Arithmetic with comparison
bool logical = x > 0 && y == z;   // Comparison with logical operators
```

### Complex Expressions
```c
// Mixed arithmetic and logical operations
bool result = a + b >= c * 2 && flag;
bool check = (x > 0) || (y < 10 && z != 0);
int calc = ++counter * value-- + offset;

// Chained comparisons and operations
bool range_check = value >= min && value <= max;
bool valid = score > 90 || (attempts < 3 && bonus_points > 0);
int formula = base + increment++ * multiplier;
```

### Operator Precedence (Highest to Lowest)
```c
// 1. Primary expressions: literals, identifiers, parentheses
// 2. Postfix: x++, x--
// 3. Unary/Prefix: !x, ++x, --x, -x
// 4. Multiplicative: *, /, %
// 5. Additive: +, -
// 6. Comparison: <, >, <=, >=
// 7. Equality: ==, !=
// 8. Logical AND: &&
// 9. Logical OR: ||
// 10. Assignment: =, +=, -=, *=, /=

int example = !flag || x++ > y * 2 && z <= w + 1;
//            ^    ^    ^     ^      ^   ^
//            3    2    6     4      7   5
```

### Comments
```c
// This is a single-line comment
int x = 5; // Inline comment
// Comments can contain any text: symbols !@#$%^&*()
```

### Special Number Formats
```c
int zero = 0;           // Zero
int big_num = 123456;   // Large integers
float small = 0.5;      // Leading zero optional
float tiny = .25;       // No leading zero
float precise = 3.14159265; // High precision
```

### String Handling
```c
string empty = "";              // Empty string
string greeting = "Hello";      // Simple string
string message = "Hello World"; // String with spaces
string quote_content = "She said \"Hello\""; // Escaped quotes (future feature)
```

### Boolean Logic Examples
```c
bool user_valid = age >= 18 && has_license;
bool discount_eligible = is_student || is_senior || purchase_amount > 100;
bool access_granted = is_admin || (is_user && has_permission && !is_banned);
bool in_range = value >= 0 && value <= 100;
bool valid_score = score > 0 && score <= max_score && !is_disqualified;
```

### Type Safety
> **Note**: Type checking is currently disabled during AST generation. It will be implemented in a dedicated semantic analysis phase that runs after AST construction for better separation of concerns.
```c
int x = "hello";        // ❌ Error: Cannot assign string to int (future semantic analysis)
string name = 123;      // ❌ Error: Cannot assign int to string (future semantic analysis)
float bad = 3.14.15;    // ❌ Error: Invalid float format (detected during parsing)
bool flag = "yes";      // ❌ Error: Cannot assign string to bool (future semantic analysis)
```c
int x = "hello";        // ❌ Error: Cannot assign string to int
string name = 123;      // ❌ Error: Cannot assign int to string
float bad = 3.14.15;    // ❌ Error: Invalid float format
bool flag = "yes";      // ❌ Error: Cannot assign string to bool
float invalid = 3..14;  // ❌ Error: Multiple decimal points
int malformed = -;      // ❌ Error: Invalid expression
string broken = "hello; // ❌ Error: Missing closing quote
```

### Error Detection Examples
```c
// Syntax Errors (detected during parsing)
float bad1 = 3.14.15;   // Multiple decimals
float bad2 = 3.1a4;     // Invalid character in float
float bad3 = .;         // No digits in float
string bad4 = "hello;   // Missing closing quote

// Type Mismatches (detected during semantic analysis - when enabled)
int x = "text";         // String to int assignment
bool flag = 42;         // Int to bool assignment  
string msg = 3.14;      // Float to string assignment

// Invalid Expressions
int result = + * 5;     // Invalid operator sequence
bool check = x ++ y;    // Invalid increment usage
int bad = =5;           // Invalid assignment as value
```

## 🚀 Quick Start

### Prerequisites
- GCC compiler or Clang
- CMake (3.10 or higher)
- Git (for Unity submodule)

### Installation

1. **Clone the repository**
   ```bash
   git clone https://github.com/Blopaa/Compiler.git
   cd Compiler
   
   # Switch to development branch
   git checkout dev
   ```

2. **Add Unity testing framework**
   ```bash
   git submodule add https://github.com/ThrowTheSwitch/Unity.git test/unity
   git submodule update --init --recursive
   ```

3. **Build the compiler**
   ```bash
   mkdir build && cd build
   cmake ..
   
   # Windows (with MinGW/CLion)
   cmake --build .
   
   # Linux/macOS  
   make
   ```

4. **Run tests**
   ```bash
   ./test_runner
   
   # Or with CTest
   ctest --output-on-failure
   ```

### Basic Usage

**Interactive Mode:**
```bash
./compiler
```

The compiler processes the default example and shows the compilation pipeline:
```
Input: bool result = a + b >= c * 2;

1. SPLITTING:
split 0: 'bool'
split 1: 'result'  
split 2: '='
split 3: 'a'
split 4: '+'
split 5: 'b'
split 6: '>='
split 7: 'c'
split 8: '*'
split 9: '2'
split 10: ';'
Total tokens: 11

2. TOKENIZATION:
Token 0: 'bool', type: 4
Token 1: 'result', type: 1  
Token 2: '=', type: 0
Token 3: 'a', type: 1
Token 4: '+', type: 9
Token 5: 'b', type: 1
Token 6: '>=', type: 29
Token 7: 'c', type: 1
Token 8: '*', type: 11
Token 9: '2', type: 1
Token 10: ';', type: 5
Total tokens processed: 11

3. AST GENERATION:
AST:
└── BOOL_VAR_DEF: result
    └── GREATER_EQUAL_OP
        ├── ADD_OP
        │   ├── VARIABLE: a
        │   └── VARIABLE: b
        └── MUL_OP
            ├── VARIABLE: c
            └── INT_LIT: 2

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
| **Lexer** | Tokenizes input, handles strings, numbers, operators, comments | `src/lexer/lexer.c` |
| **Parser** | Generates AST from tokens, type checking, precedence handling | `src/parser/parser.c` |
| **Error Handler** | Comprehensive error reporting with specific codes | `src/errorHandling/errorHandling.c` |
| **Unity Tests** | Professional test suite with CI/CD integration | `test/test_main.c` |

## 🧪 Testing

The project uses the **Unity Testing Framework** for professional, industry-standard testing with excellent CI/CD integration.

### Run Tests

**Basic test execution:**
```bash
# From build directory
./test_runner

# With detailed output
ctest --output-on-failure --verbose

# Memory leak detection (Linux/macOS)
valgrind --leak-check=full ./test_runner
```

**Expected output:**
```
=== BASIC DECLARATIONS TESTS ===
test_basic_int_declaration:PASS
test_basic_string_declaration:PASS
test_basic_float_declaration:PASS
test_basic_bool_true_declaration:PASS
test_basic_bool_false_declaration:PASS

=== TOKEN COUNT TESTS ===
test_basic_int_token_count:PASS
test_simple_addition_token_count:PASS

=== ARITHMETIC OPERATIONS TESTS ===
test_simple_addition:PASS
test_simple_subtraction:PASS
test_modulo_operation:PASS

=== PRECEDENCE TESTS ===
test_precedence_multiply_first:PASS
test_complex_precedence:PASS

=== NEGATIVE NUMBERS TESTS ===
test_basic_negative_int:PASS
test_negative_in_expression:PASS

=== INCREMENT/DECREMENT TESTS ===
test_prefix_increment:PASS
test_postfix_decrement:PASS

=== COMPOUND ASSIGNMENTS TESTS ===
test_compound_add_assign:PASS

=== COMMENT TESTS ===
test_void_comment:PASS
test_comment_then_code:PASS

47 Tests 0 Failures 0 Ignored 
OK
```

### Test Categories

- **Basic Declarations**: Variable declarations with all data types
- **Token Count**: Lexer tokenization accuracy
- **Arithmetic Operations**: Mathematical expressions and precedence
- **Negative Numbers**: Negative literals vs subtraction operators
- **Increment/Decrement**: Prefix and postfix operators
- **Compound Assignments**: `+=`, `-=`, `*=`, `/=` operations
- **Comments**: Comment handling and code parsing
- **Special Values**: Edge cases like empty strings, zero values

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
│   └── parser/  
│       ├── parser.c           # AST generation and type checking
│       └── parser.h           # Parser interface and AST node types
├── test/
│   ├── unity/                 # Unity testing framework (submodule)
│   └── test_main.c           # Comprehensive Unity test suite
├── build/                    # Build directory (auto-generated)
├── CMakeLists.txt           # CMake build configuration
├── .github/workflows/       # CI/CD configuration
├── README.md               # This file
├── LICENSE.md             # Project license
├── CONTRIBUTING.md        # Contribution guidelines
└── .gitignore            # Git ignore rules
```

### Current Language Grammar

```
Program           → Statement*

Statement         → VarDecl | Assignment | CompoundAssign

VarDecl           → Type IDENTIFIER '=' Expression ';'
Assignment        → IDENTIFIER '=' Expression ';'  
CompoundAssign    → IDENTIFIER CompoundOp Expression ';'

Expression        → LogicalOrExpr
LogicalOrExpr     → LogicalAndExpr ('||' LogicalAndExpr)*
LogicalAndExpr    → EqualityExpr ('&&' EqualityExpr)*
EqualityExpr      → ComparisonExpr (('==' | '!=') ComparisonExpr)*
ComparisonExpr    → TermExpr (('<' | '>' | '<=' | '>=') TermExpr)*
TermExpr          → FactorExpr (('+' | '-') FactorExpr)*
FactorExpr        → UnaryExpr (('*' | '/' | '%') UnaryExpr)*
UnaryExpr         → ('!' | '++' | '--') UnaryExpr
                  | PrimaryExpr ('++' | '--')?
                  | PrimaryExpr
PrimaryExpr       → NUMBER | FLOAT | STRING | BOOLEAN | IDENTIFIER
                  | '-' NUMBER | '-' FLOAT
                  | '(' Expression ')'

Type              → 'int' | 'float' | 'string' | 'bool'
CompoundOp        → '+=' | '-=' | '*=' | '/='
BOOLEAN           → 'true' | 'false'
NUMBER            → DIGIT+ | '-' DIGIT+
FLOAT             → DIGIT* '.' DIGIT+ | '-' DIGIT* '.' DIGIT+
STRING            → '"' CHAR* '"'
IDENTIFIER        → LETTER (LETTER | DIGIT | '_')*
DIGIT             → [0-9]
LETTER            → [a-zA-Z]
CHAR              → any character except '"'
```

### Unity Testing Integration

The project uses **Unity**, the most popular C testing framework, providing:

- ✅ **Professional assertions**: `TEST_ASSERT_EQUAL_INT()`, `TEST_ASSERT_NOT_NULL()`
- ✅ **Detailed failure reports**: Line numbers, expected vs actual values
- ✅ **CI/CD integration**: GitHub Actions, Jenkins, Travis CI support
- ✅ **Industry standard**: Used by thousands of C projects
- ✅ **Memory management**: Built-in setUp/tearDown hooks
- ✅ **Cross-platform**: Windows, Linux, macOS compatibility

### Adding New Features

1. **New Token Types**: Update `TokenType` enum in `src/lexer/lexer.h`
2. **New AST Nodes**: Add to `NodeTypes` in `src/parser/parser.h`
3. **Error Types**: Extend `ErrorCode` in `src/errorHandling/errorHandling.h`
4. **Parser Logic**: Update parsing functions in `src/parser/parser.c`
5. **Tests**: Add Unity test cases in `test/test_main.c`

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
- [x] Professional Unity testing framework with CI/CD

**Phase 2: Control Flow**
- [ ] **Conditional Statements**: `if/else` statements
- [ ] **Loops**: `while`, `for` statements
- [x] **Comparison Operators**: `==`, `!=`, `<`, `>`, `<=`, `>=`
- [x] **Logical Operators**: `&&`, `||`, `!`
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
2. **Clone with submodules**: `git clone --recursive <your-fork>`
3. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
4. **Add Unity tests** for your changes in `test/test_main.c`
5. **Ensure** all tests pass (`./test_runner`)
6. **Commit** your changes (`git commit -m 'Add amazing feature'`)
7. **Push** to the branch (`git push origin feature/amazing-feature`)
8. **Open** a Pull Request

### Contribution Guidelines
- ✅ Write Unity tests for new features
- ✅ Follow existing code style
- ✅ Update documentation
- ✅ Keep commits atomic and well-described
- ✅ Ensure error handling for edge cases
- ✅ All tests must pass in CI/CD pipeline

### Adding Tests

Add new tests to `test/test_main.c` using Unity framework:

```c
void test_your_new_feature(void) {
    Input res = splitter("your test input;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);
    
    TEST_ASSERT_NOT_NULL(ast);
    TEST_ASSERT_FALSE(hasErrors());
    
    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

// Add to main():
RUN_TEST(test_your_new_feature);
```

## 🔧 CI/CD Integration

The project includes GitHub Actions workflows for:

- ✅ **Multi-compiler testing** (GCC, Clang)
- ✅ **Cross-platform builds** (Linux, macOS, Windows)
- ✅ **Memory leak detection** with Valgrind
- ✅ **Automated test reports**
- ✅ **Unity framework integration**

See `.github/workflows/ci.yml` for complete configuration.

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
4. **Unity Testing**: Professional, industry-standard test framework
5. **Modular Design**: Each phase is independent and testable
6. **CI/CD Ready**: Automated testing and deployment

<div align="center">

**⭐ If you found this helpful, please consider giving it a star! ⭐**

[Report Bug](https://github.com/Blopaa/Compiler/issues) • [Request Feature](https://github.com/Blopaa/Compiler/issues) • [Ask Question](https://github.com/Blopaa/Compiler/discussions)

</div>
