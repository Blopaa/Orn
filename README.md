# Orn Lang

*A modern low-level programming language with clear error messages and fast builds*

<p align="center">
  <img src="assets/orn.png" alt="Orn Lang Logo" width="120">
</p>
<p align="center">
  <a href="#why">Why?</a> •
  <a href="#language-goals">Goals</a> •
  <a href="#project-status">Status</a> •
  <a href="#getting-started">Getting Started</a> •
  <a href="#usage">Usage</a>
</p>

---

## Introduction

Orn is a strongly typed, C-like programming language designed for performance and clarity. Its primary goals are **fast single-pass compilation**, **precise diagnostics**, and a **clean, maintainable architecture**.
While the current design uses a C-style syntax, the long-term vision is to evolve into a fully **object-oriented language**.

---

## Why?

Many existing compilers trade developer experience for performance, or vice versa. Orn aims to balance both by offering:

* **Readable diagnostics** inspired by Rust’s error system
* **Fast compile times** with a lightweight, single-pass pipeline
* **Strong type guarantees** to prevent runtime surprises
* **A foundation that can grow into an OOP-first language**

---

## Language Goals

* **🎯 Clear Diagnostics** – Errors that are easy to understand and fix
* **⚡ Fast Compilation** – Designed for quick iteration cycles
* **🔒 Type Safety** – Strong typing at the core
* **🚀 Path to OOP** – Current C-like syntax is just the first step
* **🛠️ Simplicity First** – Minimalism without sacrificing power

---

## Project Status

**Current Phase:** Beta

Orn can already generate x86-64 assembly with working type checking and semantic analysis. Development is focused on extending language features and preparing the ground for future OOP support.

**Implemented Features:**

* Lexer with multi-line comment support
* Full parser with operator precedence
* Type checker and semantic analysis
* x86-64 code generation
* Detailed error reporting
* Functions with definitions and calls

**In Progress:**

* Optimization of generated assembly
* Built-in functions and libraries
* Debugging features
* OOP feature design

---

## Getting Started

### Prerequisites

You’ll need:

* **[GCC](https://gcc.gnu.org/)** or **[Clang](https://clang.llvm.org/)** – C compiler
* **[CMake](https://cmake.org/)** (3.10+) – Build system
* **[Git](https://git-scm.com/)** – Version control
* **[Valgrind](https://valgrind.org/)** *(optional)* – Memory debugging

### Installation

```bash
# Clone the repository
git clone https://github.com/Blopaa/Orn.git
cd Orn

# Build the project
mkdir build && cd build
cmake ..
cmake --build . --target run_program
```

---

## Usage

### Example Program

```c
int x = 42;
float rate = 3.14;
string msg = "Hello, World!";
bool b = true; :: bools and ints cannot mix

x > 0 ? {
   print(msg);
}

while x > 0 {
    print(x);
     x = x - 1;
}

:: simple add function
fn add (a: int, b: int) -> int {
    return a + b;
};

print(add(3, 5));
  
```

### Error Example

Orn provides actionable error messages:

```
error [E1001]: mismatched types (x)
  --> source.c:2:11
   |
   2 | int x = "hello";
       |           ^
   |
   = expected `int`, found `string`
   = note: string literals cannot be assigned to int variables
   = suggestion: change variable type or cast the value
```

---

## Architecture

```
Source → Lexer → Parser → Type Checker → Code Generator → Assembly
         ↓        ↓         ↓              ↓
       Tokens   AST    Symbol Table   x86-64 AT&T Code
```

---

## Join Us

We welcome contributors and feedback!

* Visit the [GitHub repository](https://github.com/Blopaa/Orn)
* Check the [contribution guidelines](CONTRIBUTING.md)
* Report issues on the [issue tracker](https://github.com/Blopaa/Orn/issues)
* Explore the [roadmap](https://github.com/Blopaa/Orn/projects)
* Join our [Discord](https://discord.gg/E8qqVC9jcf)

---

### Attribution

This README’s structure and presentation were inspired by [TheDevConnor / Luma](https://github.com/TheDevConnor/luma).

---

<p align="center">
  <strong>Built with ❤️ </strong>
</p>
