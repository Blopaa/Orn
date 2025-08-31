# 🚀 CInterpreter

A minimal, educational C interpreter/compiler written in C that demonstrates the fundamental stages of compilation: **lexical analysis**, **parsing**, and **AST generation**.

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/Blopaa/CInterpreter)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Language](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Branch](https://img.shields.io/badge/branch-dev-orange.svg)](https://github.com/Blopaa/CInterpreter/tree/dev)

## ✨ Features

- 🔍 **Lexical Analysis**: Advanced tokenizer that handles strings, numbers, operators, and keywords
- 🌳 **AST Generation**: Clean Abstract Syntax Tree representation
- 🎯 **Type System**: Strong type checking with detailed error messages
- ⚡ **Expression Parsing**: Support for arithmetic expressions (`+`, `-`)
- 🛡️ **Error Handling**: Comprehensive error reporting system
- 🧪 **Testing Framework**: Built-in test suite with 95%+ coverage
- 📚 **Educational**: Perfect for learning compiler construction

## 🎯 Supported Language Features

### Data Types
```c
int age = 25;           // Integer variables
float pi = 3.14159;     // Floating-point numbers  
string name = "Alice";  // String literals
```

### Arithmetic Operations
```c
int sum = a + b;        // Addition
int diff = x - y;       // Subtraction
float result = 2.5 + 1.8;  // Mixed operations
```

### Type Safety
```c
int x = "hello";        // ❌ Error: Cannot assign string to int
string name = 123;      // ❌ Error: Cannot assign int to string
float bad = 3.14.15;    // ❌ Error: Invalid float format
```

## 🚀 Quick Start

### Prerequisites
- GCC compiler
- Make (optional, for easier building)

### Installation

1. **Clone the repository**
   ```bash
   git clone https://github.com/Blopaa/CInterpreter.git
   cd CInterpreter
   
   # Switch to development branch
   git checkout dev
   ```

2. **Build the compiler**
   ```bash
   make
   # Or manually:
   gcc -o compiler main.c lexer/lexer.c parser/parser.c errorHandling/errorHandling.c testing/testing.c
   ```

3. **Run a quick test**
   ```bash
   ./compiler --test
   ```

### Basic Usage

**Interactive Mode:**
```bash
./compiler
```

The compiler will process the default example:
```
Input: int bad = 3.14;

1. SPLITTING:
split 0: 'int'
split 1: 'bad'  
split 2: '='
split 3: '3.14'
split 4: ';'
Total tokens: 5

2. TOKENIZATION:
Token 0: 'int', type: 2
Token 1: 'bad', type: 1  
Token 2: '=', type: 0
Token 3: '3.14', type: 1
Token 4: ';', type: 5
Total tokens processed: 5

3. AST GENERATION:
AST:
└── INT_VAR_DEF: bad
    └── FLOAT_LIT: 3.14

ERROR [1005]: Cannot assign float literal to int variable - '3.14'
```

## 🏗️ Architecture

```
┌─────────────┐    ┌──────────────┐    ┌─────────────┐
│   Input     │───▶│    Lexer     │───▶│   Parser    │
│  "int x=5;" │    │ (Tokenizer)  │    │ (AST Gen)   │
└─────────────┘    └──────────────┘    └─────────────┘
                           │                    │
                           ▼                    ▼
                   ┌──────────────┐    ┌─────────────┐
                   │   Tokens     │    │     AST     │
                   │ [int,x,=,5,;]│    │    Tree     │
                   └──────────────┘    └─────────────┘
```

### Core Components

| Component | Description | File |
|-----------|-------------|------|
| **Lexer** | Splits input into tokens | `lexer/lexer.c` |
| **Parser** | Generates AST from tokens | `parser/parser.c` |
| **Error Handler** | Manages compilation errors | `errorHandling/errorHandling.c` |
| **Testing** | Automated test suite | `testing/testing.c` |

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
✓ AST should be generated
✓ Should have no errors

Testing: Basic string declaration  
Input: string name = "Pablo";
✓ AST should be generated
✓ Should have no errors

=== TESTING ERROR CASES ===
Testing: String to int error
Input: int x = "hello";
✓ Should have errors

=== TEST SUMMARY ===
Tests run: 12
Tests passed: 12
Tests failed: 0
🎉 All tests passed!
Success rate: 100.0%
```

## 📖 Examples

<details>
<summary><b>Example 1: Variable Declarations</b></summary>

**Input:**
```c
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
```c
int sum = a + b;
int diff = x - y;
```

**AST Output:**
```
AST:
├── INT_VAR_DEF: sum
│   └── ADD_OP
│       ├── VARIABLE: a
│       └── VARIABLE: b
└── INT_VAR_DEF: diff
    └── SUB_OP
        ├── VARIABLE: x
        └── VARIABLE: y
```
</details>

## 🛠️ Development

### Project Structure
```
simple-compiler/
├── main.c                 # Entry point
├── lexer/
│   ├── lexer.c            # Tokenization logic
│   └── lexer.h            # Lexer interface
├── parser/  
│   ├── parser.c           # AST generation
│   └── parser.h           # Parser interface
├── errorHandling/
│   ├── errorHandling.c    # Error management
│   └── errorHandling.h    # Error definitions
├── testing/
│   ├── testing.c          # Test framework
│   └── testing.h          # Test interface
├── Makefile              # Build system
└── README.md            # This file
```

### Adding New Features

1. **New Token Types**: Update `TokenType` enum in `lexer/lexer.h`
2. **New Operations**: Add to `NodeTypes` in `parser/parser.h`
3. **Error Types**: Extend `ErrorCode` in `errorHandling/errorHandling.h`
4. **Tests**: Add cases in `testing/testing.c`

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

## 🗺️ Roadmap

- [ ] **Control Flow**: `if/else`, `while`, `for` statements
- [ ] **Functions**: Function declarations and calls
- [ ] **Advanced Types**: Arrays, structs
- [ ] **Code Generation**: Generate assembly or bytecode
- [ ] **Optimization**: Basic compiler optimizations
- [ ] **Standard Library**: Built-in functions
- [ ] **IDE Integration**: Language server protocol support

## 🎓 Educational Value

This compiler is perfect for:

- 📚 **Computer Science Students** learning compiler design
- 👨‍🏫 **Educators** teaching compilation principles
- 🔬 **Researchers** prototyping language features
- 🧠 **Self-learners** understanding how compilers work

### Learning Resources

- [Crafting Interpreters](https://craftinginterpreters.com/) - Excellent book on language implementation
- [Compiler Design Course](https://www.coursera.org/learn/compilers) - Stanford's compiler course
- [LLVM Tutorial](https://llvm.org/docs/tutorial/) - Advanced code generation

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Inspired by classic compiler textbooks and courses
- Built with educational purposes in mind
- Thanks to the open-source community for inspiration

---

<div align="center">

**⭐ If you found this helpful, please consider giving it a star! ⭐**

[Report Bug](https://github.com/Blopaa/CInterpreter/issues) • [Request Feature](https://github.com/Blopaa/CInterpreter/issues) • [Ask Question](https://github.com/Blopaa/CInterpreter/discussions)

</div>