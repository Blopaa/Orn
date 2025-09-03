# Contributing to Compiler

Thank you for your interest in contributing to this educational C compiler project! We welcome contributions from developers of all skill levels.

## Ways to Contribute

- **Bug Reports**: Found a bug? Let us know!
- **Feature Requests**: Have ideas for new language features?
- **Documentation**: Improve README, comments, or add examples
- **Testing**: Add more Unity test cases or improve existing ones
- **Features**: Implement new compiler features
- **Refactoring**: Improve code quality and organization

## Getting Started

### 1. Fork and Clone

```bash
# Fork the repository on GitHub
git clone --recursive https://github.com/your-username/Compiler.git
cd Compiler

# Switch to development branch
git checkout dev
```

**Note**: Use `--recursive` to automatically clone Unity testing framework.

### 2. Set Up Development Environment

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install gcc cmake valgrind cppcheck

# macOS with Homebrew
brew install cmake valgrind cppcheck

# Windows (with vcpkg or CLion)
# Install Visual Studio with C++ support and CMake
```

### 3. Set Up Unity Testing Framework

If you didn't clone recursively:

```bash
# Add Unity as submodule
git submodule add https://github.com/ThrowTheSwitch/Unity.git test/unity
git submodule update --init --recursive
```

### 4. Build the Project

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the project
cmake --build .
# Or on Linux/macOS: make

# Test the build
./test_runner
```

### 5. Create a Branch

```bash
git checkout -b feature/your-feature-name
# or
git checkout -b fix/bug-description
```

## Development Guidelines

### Code Style

We follow a consistent coding style. Please:

- **Use 4 spaces** for indentation (no tabs)
- **Follow Linux kernel style** for braces and formatting
- **Use descriptive variable names**
- **Add comments** for complex logic
- **Keep functions small** and focused

### Testing Requirements

All contributions must include Unity tests:

- **New features**: Add test cases in `test/test_main.c`
- **Bug fixes**: Include tests that reproduce the bug
- **Ensure all tests pass**: Run `./test_runner` before submitting

**Example Unity test case:**
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

// Add to main() function:
RUN_TEST(test_your_new_feature);
```

### Memory Management

Since this is a C project:

- **Always free allocated memory**
- **Check for memory leaks**: `valgrind ./test_runner`
- **Initialize pointers to NULL**
- **Validate input parameters**

## Project Architecture

Understanding the codebase:

```
Compilation Pipeline:
Input → Lexer → Parser → AST → [Future: Code Gen]
```

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
│   └── parser/  
│       ├── parser.c           # AST generation and type checking
│       └── parser.h           # Parser interface
├── test/
│   ├── unity/                 # Unity testing framework (submodule)
│   └── test_main.c           # Unity test suite
├── build/                    # Build directory (auto-generated)
├── CMakeLists.txt           # CMake build configuration
├── CONTRIBUTING.md          # This file
├── LICENSE.md              # Project license
├── README.md              # Project documentation
└── .gitignore            # Git ignore rules
```

### Key Components

| Component | Purpose | Files |
|-----------|---------|-------|
| **Lexer** | Tokenize input | `src/lexer/lexer.c`, `src/lexer/lexer.h` |
| **Parser** | Generate AST | `src/parser/parser.c`, `src/parser/parser.h` |
| **Errors** | Handle compilation errors | `src/errorHandling/` |
| **Unity Tests** | Professional testing framework | `test/test_main.c`, `test/unity/` |

### Adding New Features

#### 1. New Token Type

1. Add to `TokenType` enum in `src/lexer/lexer.h`
2. Update `tokenMapping` array
3. Modify `splitter()` if needed
4. **Add Unity tests** in `test/test_main.c`

#### 2. New AST Node Type

1. Add to `NodeTypes` enum in `src/parser/parser.h`
2. Update `TypeDefs` mapping if applicable
3. Modify `ASTGenerator()` logic
4. Update `printASTTree()` for display
5. **Add Unity tests**

#### 3. New Error Type

1. Add to `ErrorCode` enum in `src/errorHandling/errorHandling.h`
2. Add entry to `errorList` array
3. Use `repError()` where appropriate
4. **Test error conditions with Unity**

## Unity Testing Strategy

### Test Categories

1. **Basic Cases**: Valid syntax that should compile
2. **Error Cases**: Invalid syntax that should fail gracefully (when type checking is enabled)
3. **Edge Cases**: Boundary conditions
4. **Integration Tests**: Full compilation pipeline

### Writing Good Unity Tests

```c
void test_example_feature(void) {
    // Test valid case - should succeed
    Input res = splitter("int x = 5;");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);
    
    TEST_ASSERT_NOT_NULL(ast);           // AST should be created
    TEST_ASSERT_FALSE(hasErrors());       // No compilation errors
    
    // Clean up memory
    freeInput(res);
    freeTokenList(tokens);
    freeAST(ast);
}

void test_token_counting(void) {
    // Test tokenization accuracy
    Input res = splitter("int sum = a + b;");
    
    TEST_ASSERT_EQUAL_INT(7, res->n);    // Exact token count
    
    freeInput(res);
}

void test_error_case(void) {
    // Test error detection (when type checking enabled)
    Input res = splitter("invalid syntax here");
    Token tokens = tokenization(res);
    ASTNode ast = ASTGenerator(tokens);
    
    TEST_ASSERT_TRUE(hasErrors());        // Should detect errors
    
    freeInput(res);
    freeTokenList(tokens);
    if (ast) freeAST(ast);
}
```

### Running Tests

```bash
# From build directory
./test_runner

# With detailed output
ctest --output-on-failure --verbose

# Check for memory leaks
valgrind --leak-check=full ./test_runner

# Static analysis (if available)
cppcheck --enable=all ../src/
```

### Unity Assertions Reference

```c
// Basic assertions
TEST_ASSERT_TRUE(condition)
TEST_ASSERT_FALSE(condition)
TEST_ASSERT(condition)

// Null/Not Null
TEST_ASSERT_NULL(pointer)
TEST_ASSERT_NOT_NULL(pointer)

// Integer comparisons
TEST_ASSERT_EQUAL_INT(expected, actual)
TEST_ASSERT_NOT_EQUAL_INT(expected, actual)
TEST_ASSERT_GREATER_THAN_INT(threshold, actual)

// String comparisons
TEST_ASSERT_EQUAL_STRING(expected, actual)
TEST_ASSERT_EQUAL_CHAR_ARRAY(expected, actual, num_elements)

// Float comparisons
TEST_ASSERT_EQUAL_FLOAT(expected, actual)
TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual)

// Custom messages
TEST_ASSERT_TRUE_MESSAGE(condition, "Custom failure message")
TEST_ASSERT_EQUAL_INT_MESSAGE(expected, actual, "Numbers should match")
```

## Commit Guidelines

### Commit Message Format

```
<type>(<scope>): <description>

[optional body]

[optional footer]
```

### Types
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `test`: Adding or updating Unity tests
- `refactor`: Code refactoring
- `style`: Code style changes
- `perf`: Performance improvements
- `build`: Build system changes

### Examples

```bash
feat(lexer): add support for hexadecimal integers

- Add HEX_LITERAL token type
- Update tokenization logic
- Include comprehensive Unity tests
- Document hex format in README

Fixes #23
```

```bash
test(parser): add Unity tests for increment operators

- Add test_prefix_increment() 
- Add test_postfix_decrement()
- Verify AST node generation
- Ensure memory cleanup

Closes #45
```

## Code Review Process

### Before Submitting PR

- [ ] All Unity tests pass (`./test_runner`)
- [ ] Code builds without warnings
- [ ] No memory leaks (`valgrind ./test_runner`)
- [ ] Documentation updated
- [ ] New Unity tests added for changes
- [ ] Clear commit messages

### PR Description Template

```markdown
## What does this PR do?

Brief description of changes...

## Type of Change

- [ ] Bug fix
- [ ] New feature
- [ ] Documentation update
- [ ] Unity test improvements
- [ ] Code refactoring

## Testing

- [ ] Added new Unity tests
- [ ] All existing tests pass
- [ ] Manual testing performed
- [ ] Memory leak testing completed

## Screenshots/Examples

If applicable, add examples or screenshots...

## Checklist

- [ ] Code follows project style guidelines
- [ ] Self-review completed
- [ ] Comments added for complex code
- [ ] Documentation updated
- [ ] No breaking changes (or documented)
- [ ] Unity tests cover new functionality
```

## Development Workflow

1. **Create Issue**: Discuss feature/bug before coding
2. **Fork & Branch**: Work on dedicated feature branch
3. **Code & Test**: Implement with comprehensive Unity tests
4. **Review**: Self-review and test thoroughly
5. **Submit PR**: Clear description and passing tests
6. **Iterate**: Address review feedback
7. **Merge**: Squash merge into main branch

## Build System Notes

This project uses **CMake** as its build system. Key commands:

```bash
# Clean build
rm -rf build
mkdir build && cd build
cmake ..
cmake --build .

# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .

# Release build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .

# Verbose build (see compilation commands)
cmake --build . --verbose
```

### Adding New Files

When adding new `.c` files, update `CMakeLists.txt`:

```cmake
# Add your new source file to LIB_SOURCES
set(LIB_SOURCES
    src/lexer/lexer.c
    src/parser/parser.c
    src/errorHandling/errorHandling.c
    src/your_new_file.c  # Add here
)
```

### Unity Integration

The CMakeLists.txt automatically detects Unity and builds tests:

```cmake
# Unity is detected and linked automatically
if(EXISTS "${CMAKE_SOURCE_DIR}/test/unity/src/unity.c")
    # Unity library and test_runner are created
    add_executable(test_runner test/test_main.c)
    target_link_libraries(test_runner compiler_lib unity)
endif()
```

## CI/CD Integration

### GitHub Actions

The project includes automated CI/CD that:

- ✅ Builds on multiple compilers (GCC, Clang)
- ✅ Tests on multiple platforms (Linux, macOS)
- ✅ Runs Unity test suite
- ✅ Checks for memory leaks with Valgrind
- ✅ Reports test results

### Local CI Testing

Test your changes like CI does:

```bash
# Test with different compilers
CC=gcc cmake .. && cmake --build . && ./test_runner
CC=clang cmake .. && cmake --build . && ./test_runner

# Memory leak detection
valgrind --leak-check=full --error-exitcode=1 ./test_runner

# Static analysis
cppcheck --enable=all --error-exitcode=1 ../src/
```

## Learning Resources

New to compiler development or Unity testing? Start here:

### Compiler Development
- **Books**:
  - "Crafting Interpreters" by Robert Nystrom
  - "Modern Compiler Implementation in C" by Andrew Appel

- **Online Courses**:
  - Stanford CS143: Compilers
  - Coursera: Compilers Course

### Unity Testing Framework
- **Official Docs**: [Unity Documentation](https://github.com/ThrowTheSwitch/Unity)
- **Examples**: Check `test/unity/examples/`
- **Best Practices**: Unity testing patterns and conventions

### C Development
- **Tutorials**:
  - [Let's Build a Compiler](https://compilers.iecc.com/crenshaw/)
  - [Compiler Tutorial in C](https://github.com/DoctorWkt/acwj)

## Questions?

- **Discussions**: Use GitHub Discussions for questions
- **Issues**: Report bugs or request features
- **Contact**: Reach out to maintainers

## Recognition

Contributors are recognized in:

- GitHub contributors graph
- Release notes for significant contributions
- README acknowledgments for major features

## Code of Conduct

We're committed to providing a welcoming environment:

- **Be respectful** and inclusive
- **Provide constructive feedback**
- **Help others learn** and grow
- **Focus on the code**, not the person

---

Thank you for contributing to Compiler!

Every contribution, no matter how small, helps make this project better for the community.

### Quick Checklist for Contributors

Before submitting your PR:

- [ ] Unity submodule is properly initialized
- [ ] All Unity tests pass (`./test_runner`)
- [ ] New features have corresponding Unity tests
- [ ] Memory management is handled correctly
- [ ] Code follows project style guidelines
- [ ] Documentation is updated
- [ ] Commit messages are clear and descriptive