# 🤝 Contributing to CInterpreter

Thank you for your interest in contributing to this educational C interpreter/compiler project! We welcome contributions from developers of all skill levels.

## 🎯 Ways to Contribute

- 🐛 **Bug Reports**: Found a bug? Let us know!
- 💡 **Feature Requests**: Have ideas for new language features?
- 📚 **Documentation**: Improve README, comments, or add examples
- 🧪 **Testing**: Add more test cases or improve existing ones
- 🚀 **Features**: Implement new compiler features
- 🔧 **Refactoring**: Improve code quality and organization

## 🚀 Getting Started

### 1. Fork and Clone

```bash
# Fork the repository on GitHub
git clone https://github.com/your-username/CInterpreter.git
cd CInterpreter

# Switch to development branch
git checkout dev
```

### 2. Set Up Development Environment

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install gcc make valgrind cppcheck astyle

# Build the project
make

# Run tests to ensure everything works
make test
```

### 3. Create a Branch

```bash
git checkout -b feature/your-feature-name
# or
git checkout -b fix/bug-description
```

## 📋 Development Guidelines

### Code Style

We follow a consistent coding style. Please:

- **Use 4 spaces** for indentation (no tabs)
- **Follow Linux kernel style** for braces and formatting
- **Use descriptive variable names**
- **Add comments** for complex logic
- **Keep functions small** and focused

**Auto-format your code:**
```bash
make style
```

### Testing Requirements

All contributions must include tests:

- **New features**: Add test cases in `testing/testing.c`
- **Bug fixes**: Include tests that reproduce the bug
- **Ensure all tests pass**: Run `make test` before submitting

**Example test case:**
```c
void testNewFeature(void) {
    printf("=== TESTING NEW FEATURE ===\n");
    testCase("Feature description", "input code", shouldPass);
}
```

### Memory Management

Since this is a C project:

- **Always free allocated memory**
- **Check for memory leaks**: `make memcheck`
- **Initialize pointers to NULL**
- **Validate input parameters**

## 🏗️ Project Architecture

Understanding the codebase:

```
Compilation Pipeline:
Input → Lexer → Parser → AST → [Future: Code Gen]
```

### Key Components

| Component | Purpose | Files |
|-----------|---------|-------|
| **Lexer** | Tokenize input | `lexer/lexer.c`, `lexer/lexer.h` |
| **Parser** | Generate AST | `parser/parser.c`, `parser/parser.h` |
| **Errors** | Handle compilation errors | `errorHandling/` |
| **Tests** | Automated testing | `testing/` |

### Adding New Features

#### 1. New Token Type

1. Add to `TokenType` enum in `lexer/lexer.h`
2. Update `tokenMapping` array
3. Modify `splitter()` if needed
4. Add tests

#### 2. New AST Node Type

1. Add to `NodeTypes` enum in `parser/parser.h`
2. Update `TypeDefs` mapping if applicable
3. Modify `ASTGenerator()` logic
4. Update `printASTTree()` for display
5. Add tests

#### 3. New Error Type

1. Add to `ErrorCode` enum in `errorHandling/errorHandling.h`
2. Add entry to `errorList` array
3. Use `repError()` where appropriate
4. Test error conditions

## 🧪 Testing Strategy

### Test Categories

1. **Basic Cases**: Valid syntax that should compile
2. **Error Cases**: Invalid syntax that should fail gracefully
3. **Edge Cases**: Boundary conditions
4. **Integration Tests**: Full compilation pipeline

### Writing Good Tests

```c
void testExample(void) {
    // Test valid case
    testCase("Valid float declaration", 
             "float pi = 3.14;", 
             1);  // Should pass
    
    // Test error case
    testCase("Invalid float format", 
             "float bad = 3.14.15;", 
             0);  // Should fail
}
```

### Running Tests

```bash
make test           # Run all tests
make memcheck       # Check for memory leaks
make analyze        # Static code analysis  
make coverage       # Generate coverage report
```

## 📝 Commit Guidelines

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
- `test`: Adding or updating tests
- `refactor`: Code refactoring
- `style`: Code style changes
- `perf`: Performance improvements

### Examples

```bash
feat(lexer): add support for hexadecimal integers

- Add HEX_LITERAL token type
- Update tokenization logic
- Include comprehensive tests
- Document hex format in README

Fixes #23
```

```bash
fix(parser): handle empty expression in variable assignment

- Add null check before creating value node
- Improve error message for empty assignments
- Add regression test

Closes #45
```

## 🔍 Code Review Process

### Before Submitting PR

- [ ] ✅ All tests pass (`make test`)
- [ ] 🧹 Code is formatted (`make style`)
- [ ] 🔍 No memory leaks (`make memcheck`)
- [ ] 📚 Documentation updated
- [ ] 🧪 New tests added for changes
- [ ] 💬 Clear commit messages

### PR Description Template

```markdown
## What does this PR do?

Brief description of changes...

## Type of Change

- [ ] 🐛 Bug fix
- [ ] ✨ New feature
- [ ] 📚 Documentation update
- [ ] 🧪 Test improvements
- [ ] 🔧 Code refactoring

## Testing

- [ ] Added new tests
- [ ] All existing tests pass
- [ ] Manual testing performed

## Screenshots/Examples

If applicable, add examples or screenshots...

## Checklist

- [ ] Code follows project style guidelines
- [ ] Self-review completed
- [ ] Comments added for complex code
- [ ] Documentation updated
- [ ] No breaking changes (or documented)
```

## 🚦 Development Workflow

1. **Create Issue**: Discuss feature/bug before coding
2. **Fork & Branch**: Work on dedicated feature branch
3. **Code & Test**: Implement with comprehensive tests
4. **Review**: Self-review and test thoroughly
5. **Submit PR**: Clear description and passing CI
6. **Iterate**: Address review feedback
7. **Merge**: Squash merge into main branch

## 🎓 Learning Resources

New to compiler development? Start here:

- **Books**:
    - "Crafting Interpreters" by Robert Nystrom
    - "Modern Compiler Implementation in C" by Andrew Appel

- **Online Courses**:
    - Stanford CS143: Compilers
    - Coursera: Compilers Course

- **Tutorials**:
    - [Let's Build a Compiler](https://compilers.iecc.com/crenshaw/)
    - [Compiler Tutorial in C](https://github.com/DoctorWkt/acwj)

## 🤔 Questions?

- 💬 **Discussions**: Use GitHub Discussions for questions
- 🐛 **Issues**: Report bugs or request features
- 📧 **Contact**: Reach out to maintainers

## 🏆 Recognition

Contributors are recognized in:

- GitHub contributors graph
- Release notes for significant contributions
- README acknowledgments for major features

## 📜 Code of Conduct

We're committed to providing a welcoming environment:

- **Be respectful** and inclusive
- **Provide constructive feedback**
- **Help others learn** and grow
- **Focus on the code**, not the person

---

Thank you for contributing to CInterpreter! 🙏

Every contribution, no matter how small, helps make this project better for the community.