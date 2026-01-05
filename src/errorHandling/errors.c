#include "errorHandling.h"

const ErrorInfo errorDatabase[] = {
    // Type mismatch errors (1000s)
    {
        ERROR_TYPE_MISMATCH_DOUBLE_TO_INT,
        ERROR,
        "mismatched types",
        "double literals cannot be assigned to int variables",
        "expected `int`, found `double`",
        "try truncating the decimal or change the variable to `double`"
    },
    {
        ERROR_TYPE_MISMATCH_DOUBLE_TO_FLOAT,
        WARNING,
        "mismatched types",
        "double literals may lose data when assigned to float variables",
        "expected `float`, found `double` - possible precision loss or infinite float",
        "try casting to float or change the variable to `double`"
    },
    {
        ERROR_TYPE_MISMATCH_STRING_TO_DOUBLE,
        ERROR,
        "mismatched types",
        "string literals cannot be assigned to double variables",
        "expected `double`, found `string`",
        "try converting the string to a double or change the variable type"
    },
    {
        ERROR_TYPE_MISMATCH_BOOL_TO_DOUBLE,
        ERROR,
        "mismatched types",
        "boolean values cannot be assigned to double variables",
        "expected `double`, found `bool`",
        "try using conditional assignment or change the variable type"
    },
    {
        ERROR_TYPE_MISMATCH_DOUBLE_TO_BOOL,
        ERROR,
        "mismatched types",
        "double literals cannot be assigned to bool variables",
        "expected `bool`, found `double`",
        "try using a comparison like `value != 0.0` or `true`/`false`"
    },
    {
        ERROR_TYPE_MISMATCH_DOUBLE_TO_STRING,
        ERROR,
        "mismatched types",
        "double literals cannot be assigned to string variables",
        "expected `string`, found `double`",
        "try wrapping the number in quotes or change the variable type"
    },
    {
        ERROR_TYPE_MISMATCH_STRING_TO_INT,
        ERROR,
        "mismatched types",
        "string literals cannot be assigned to int variables",
        "expected `int`, found `string`",
        "try converting the string to an integer or change the variable type"
    },
    {
        ERROR_TYPE_MISMATCH_INT_TO_STRING,
        ERROR,
        "mismatched types",
        "integer literals cannot be assigned to string variables",
        "expected `string`, found `int`",
        "try wrapping the number in quotes or change the variable type"
    },
    {
        ERROR_TYPE_MISMATCH_FLOAT_TO_STRING,
        ERROR,
        "mismatched types",
        "float literals cannot be assigned to string variables",
        "expected `string`, found `float`",
        "try wrapping the number in quotes or change the variable type"
    },
    {
        ERROR_TYPE_MISMATCH_STRING_TO_FLOAT,
        ERROR,
        "mismatched types",
        "string literals cannot be assigned to float variables",
        "expected `float`, found `string`",
        "try converting the string to a float or change the variable type"
    },
    {
        ERROR_TYPE_MISMATCH_FLOAT_TO_INT,
        ERROR,
        "mismatched types",
        "float literals lose precision when assigned to int variables",
        "expected `int`, found `float`",
        "try truncating the decimal or change the variable to `float`"
    },
    {
        ERROR_TYPE_MISMATCH_STRING_TO_BOOL,
        ERROR,
        "mismatched types",
        "string literals cannot be assigned to bool variables",
        "expected `bool`, found `string`",
        "try using `true`/`false` or change the variable type"
    },
    {
        ERROR_TYPE_MISMATCH_FLOAT_TO_BOOL,
        ERROR,
        "mismatched types",
        "float literals cannot be assigned to bool variables",
        "expected `bool`, found `float`",
        "try using a comparison like `value != 0.0` or `true`/`false`"
    },
    {
        ERROR_TYPE_MISMATCH_INT_TO_BOOL,
        ERROR,
        "mismatched types",
        "integer values cannot be directly assigned to bool variables",
        "expected `bool`, found `int`",
        "try using a comparison like `value != 0` or `true`/`false`"
    },
    {
        ERROR_TYPE_MISMATCH_BOOL_TO_STRING,
        ERROR,
        "mismatched types",
        "boolean values cannot be assigned to string variables",
        "expected `string`, found `bool`",
        "try using conditional assignment or change the variable type"
    },
    {
        ERROR_TYPE_MISMATCH_BOOL_TO_INT,
        ERROR,
        "mismatched types",
        "boolean values cannot be directly assigned to int variables",
        "expected `int`, found `bool`",
        "try using conditional assignment or change variable type"
    },
    {
        ERROR_TYPE_MISMATCH_BOOL_TO_FLOAT,
        ERROR,
        "mismatched types",
        "boolean values cannot be assigned to float variables",
        "expected `float`, found `bool`",
        "try using conditional assignment or change the variable type"
    },
    {
        ERROR_INCOMPATIBLE_BINARY_OPERANDS,
        ERROR,
        "incompatible types for binary operation",
        "operands have different types that cannot be used together",
        "type mismatch in binary expression",
        "ensure both operands have compatible types or cast them appropriately"
    },
    {
        ERROR_VOID_IN_EXPRESSION,
        ERROR,
        "void value in expression",
        "void functions cannot be used in expressions",
        "void type has no value",
        "call the function separately if you need its side effects"
    },

    // Variable/Symbol errors (2000s)
    {
        ERROR_UNDEFINED_VARIABLE,
        ERROR,
        "cannot find value in this scope",
        "this variable has not been declared",
        "variable not found",
        "check the variable name or declare it before use"
    },
    {
        ERROR_VARIABLE_REDECLARED,
        ERROR,
        "variable redeclared",
        "a variable with this name already exists in the current scope",
        "identifier already declared",
        "use a different name or remove one of the declarations"
    },
    {
        ERROR_VARIABLE_NOT_INITIALIZED,
        WARNING,
        "variable used before being assigned",
        "uninitialized variables contain unpredictable values",
        "initialize the variable before use",
        "add an initializer when declaring the variable"
    },
    {
        ERROR_INVALID_VARIABLE_NAME,
        ERROR,
        "invalid variable name",
        "variable names must follow naming conventions",
        "invalid identifier",
        "use letters, digits, and underscores; start with letter or underscore"
    },
    {
        ERROR_CONSTANT_REASSIGNMENT,
        ERROR,
        "cannot assign to constant",
        "constants cannot be modified after initialization",
        "assignment to immutable value",
        "use a mutable variable instead"
    },
    {
        ERROR_UNDEFINED_SYMBOL,
        ERROR,
        "undefined symbol",
        "this symbol has not been declared in any accessible scope",
        "symbol not found",
        "check the spelling or import the required module"
    },
    {
        ERROR_SYMBOL_NOT_VARIABLE,
        ERROR,
        "symbol is not a variable",
        "attempting to use a non-variable symbol as a variable",
        "invalid symbol usage",
        "this symbol might be a function or type name"
    },

    // Syntax errors (3000s)
    {
        ERROR_INVALID_FLOAT_MULTIPLE_DECIMALS,
        ERROR,
        "invalid float literal",
        "float literals can only contain one decimal point",
        "remove extra decimal points",
        "use a valid float format like `3.14`"
    },
    {
        ERROR_INVALID_FLOAT_INVALID_CHAR,
        ERROR,
        "invalid float literal",
        "float literals can only contain digits and one decimal point",
        "invalid character in float",
        "remove non-numeric characters from the float literal"
    },
    {
        ERROR_INVALID_FLOAT_NO_DIGITS,
        ERROR,
        "invalid float literal",
        "float literals must contain at least one digit",
        "empty float literal",
        "add digits before or after the decimal point"
    },
    {
        ERROR_EXPECTED_QUOTE,
        ERROR,
        "unterminated string literal",
        "string literals must be enclosed in double quotes",
        "add the missing closing quote",
        "ensure all strings end with `\"`"
    },
    {
        ERROR_INVALID_EXPRESSION,
        ERROR,
        "invalid expression",
        "this expression cannot be parsed correctly",
        "check the syntax",
        "verify operator precedence and parentheses"
    },
    {
        ERROR_UNMATCHED_LEFT_BRACE,
        ERROR,
        "unmatched opening brace",
        "every opening brace needs a corresponding closing brace",
        "missing closing brace",
        "add a closing `}` or remove the extra opening brace"
    },
    {
        ERROR_TERNARY_MISSING_TRUE_BRANCH,
        ERROR,
        "incomplete ternary expression",
        "ternary operator requires both true and false branches",
        "missing expression after `?`",
        "add the true branch: `condition ? true_value : false_value`"
    },
    {
        ERROR_TERNARY_MISSING_FALSE_BRANCH,
        ERROR,
        "incomplete ternary expression",
        "ternary operator requires both true and false branches",
        "missing expression after `:`",
        "add the false branch: `condition ? true_value : false_value`"
    },
    {
        ERROR_TERNARY_INVALID_CONDITION,
        ERROR,
        "invalid ternary condition",
        "ternary condition must be a boolean expression",
        "condition is not boolean",
        "use a comparison or boolean expression before `?`"
    },
    {
        ERROR_EXPECTED_SEMICOLON,
        ERROR,
        "missing semicolon",
        "statements must be terminated with a semicolon",
        "expected `;`",
        "add a semicolon at the end of the statement"
    },
    {
        ERROR_UNEXPECTED_TOKEN,
        ERROR,
        "unexpected token",
        "this token was not expected in the current context",
        "syntax error",
        "check for missing operators, parentheses, or semicolons"
    },
    {
        ERROR_EXPECTED_CLOSING_PAREN,
        ERROR,
        "missing closing parenthesis",
        "opening parenthesis has no matching closing parenthesis",
        "expected `)`",
        "add the missing `)` or remove the extra `(`"
    },
    {
        ERROR_EXPECTED_OPENING_PAREN,
        ERROR,
        "missing opening parenthesis",
        "closing parenthesis has no matching opening parenthesis",
        "expected `(`",
        "add the missing `(` or remove the extra `)`"
    },
    {
        ERROR_EXPECTED_OPENING_BRACE,
        ERROR,
        "expected opening brace",
        "a block must start with an opening brace",
        "missing `{`",
        "add `{` at the beginning of the block"
    },
    {
        ERROR_EXPECTED_CLOSING_BRACE,
        ERROR,
        "expected closing brace",
        "a block must end with a closing brace",
        "missing `}`",
        "add `}` at the end of the block"
    },
    {
        ERROR_EXPECTED_QUESTION_MARK,
        ERROR,
        "expected '?' in ternary expression",
        "ternary operator requires a '?' after the condition",
        "missing '?' token",
        "write the expression as: condition ? true_value : false_value"
    },

    // Logic/Control flow errors (4000s)
    {
        ERROR_INVALID_ASSIGNMENT_TARGET,
        ERROR,
        "invalid assignment target",
        "the left side of assignment must be a variable",
        "assignment requires a variable on the left side",
        "assign to a variable, not an expression"
    },
    {
        ERROR_INVALID_OPERATION_FOR_TYPE,
        ERROR,
        "invalid operation for type",
        "this operation is not supported for the given type",
        "operation not available for this type",
        "check which operations are valid for this data type"
    },
    {
        ERROR_INCOMPATIBLE_OPERAND_TYPES,
        ERROR,
        "incompatible operand types",
        "operator cannot be applied to operands of different types",
        "type mismatch in operation",
        "ensure both operands are the same compatible type"
    },
    {
        ERROR_INVALID_UNARY_OPERAND,
        ERROR,
        "invalid unary operand",
        "unary operator cannot be applied to this type",
        "unary operation not supported",
        "check if the unary operator is valid for this data type"
    },
    {
        ERROR_UNREACHABLE_CODE,
        WARNING,
        "unreachable code detected",
        "code after return statement will never be executed",
        "dead code",
        "remove the unreachable code or restructure the control flow"
    },
    {
        ERROR_BREAK_OUTSIDE_LOOP,
        ERROR,
        "break outside loop",
        "`break` can only be used inside loop constructs",
        "invalid break statement",
        "use break only within while or for loops"
    },
    {
        ERROR_CONTINUE_OUTSIDE_LOOP,
        ERROR,
        "continue outside loop",
        "`continue` can only be used inside loop constructs",
        "invalid continue statement",
        "use continue only within while or for loops"
    },
    {
        ERROR_INVALID_LOOP_CONDITION,
        ERROR,
        "invalid loop condition",
        "loop condition must evaluate to a boolean value",
        "non-boolean loop condition",
        "use a comparison or boolean expression for the loop condition"
    },
    {
        ERROR_EXPECTED_COLON,
        ERROR,
        "expected ':'",
        "a colon is required after the parameter name in a function declaration",
        "missing ':'",
        "write the parameter as `name: type`"
    },
    {
        ERROR_EXPECTED_ARROW,
        ERROR,
        "expected '->'",
        "a function return type must be introduced with '->' after the parameter list",
        "missing '->' token",
        "write the function as `fn name(params) -> return_type { ... }`"
    },
    {
        ERROR_EXPECTED_RETURN,
        ERROR,
        "expected 'return' keyword",
        "a return statement must begin with the 'return' keyword",
        "missing 'return'",
        "write the statement as `return value;`"
    },
    {
        ERROR_EXPECTED_FN,
        ERROR,
        "expected 'fn' keyword",
        "a function declaration must begin with the 'fn' keyword",
        "missing 'fn'",
        "write the declaration as `fn name(params) -> return_type { ... }`"
    },
    {
        ERROR_INVALID_CAST_TARGET,
        ERROR,
        "invalid cast target",
        "cast target must be a valid type",
        "expected type after 'as'",
        "use a valid type like 'int', 'float', 'bool', etc."
    },
    {
        ERROR_FORBIDDEN_CAST,
        ERROR,
        "forbidden cast",
        "this cast is not allowed between these types",
        "incompatible types for casting",
        "check if explicit conversion is supported for these types"
    },
    {
        ERROR_CAST_PRECISION_LOSS,
        WARNING,
        "cast may lose precision",
        "casting from larger to smaller type may lose data",
        "potential data loss in cast",
        "ensure the value fits in the target type"
    },

    // Function-related errors (5000s)
    {
        ERROR_FUNCTION_REDEFINED,
        ERROR,
        "function redefinition",
        "a function with this name and signature already exists",
        "duplicate function definition",
        "rename the function or remove the duplicate"
    },
    {
        ERROR_UNDEFINED_FUNCTION,
        ERROR,
        "undefined function",
        "no function with this name has been declared",
        "function not found",
        "check the function name or declare it before use"
    },
    {
        ERROR_FUNCTION_ARG_COUNT_MISMATCH,
        ERROR,
        "wrong number of arguments",
        "function call has incorrect number of arguments",
        "argument count mismatch",
        "check the function signature for the correct number of parameters"
    },
    {
        ERROR_FUNCTION_ARG_TYPE_MISMATCH,
        ERROR,
        "argument type mismatch",
        "function argument has incompatible type",
        "type error in function call",
        "ensure arguments match the expected parameter types"
    },
    {
        ERROR_MISSING_RETURN_VALUE,
        ERROR,
        "missing return value",
        "non-void function must return a value",
        "expected return value",
        "add a return statement with a value of the correct type"
    },
    {
        ERROR_UNEXPECTED_RETURN_VALUE,
        ERROR,
        "unexpected return value",
        "void function cannot return a value",
        "void function with return value",
        "remove the value from the return statement"
    },
    {
        ERROR_RETURN_TYPE_MISMATCH,
        ERROR,
        "return type mismatch",
        "returned value type doesn't match function signature",
        "incompatible return type",
        "return a value of the declared return type"
    },
    {
        ERROR_INVALID_FUNCTION_NAME,
        ERROR,
        "invalid function name",
        "function names must follow naming conventions",
        "invalid identifier for function",
        "use letters, digits, and underscores; start with letter or underscore"
    },
    {
        ERROR_TOO_MANY_PARAMETERS,
        ERROR,
        "too many parameters",
        "function has more parameters than supported",
        "parameter limit exceeded",
        "reduce the number of parameters or use a struct"
    },
    {
        ERROR_DUPLICATE_PARAMETER_NAME,
        ERROR,
        "duplicate parameter name",
        "function parameters must have unique names",
        "parameter already declared",
        "use different names for each parameter"
    },
    {
        ERROR_INVALID_PARAMETER_TYPE,
        ERROR,
        "invalid parameter type",
        "parameter type is not recognized or invalid",
        "unknown parameter type",
        "use a valid type for the parameter"
    },
    {
        ERROR_CALLING_NON_FUNCTION,
        ERROR,
        "calling non-function",
        "attempting to call something that is not a function",
        "invalid function call",
        "only functions can be called with parentheses"
    },
    {
        ERROR_FUNCTION_NO_OVERLOAD_MATCH,
        ERROR,
        "no matching function overload",
        "no function overload matches the provided arguments",
        "overload resolution failed",
        "check argument types and count against available overloads"
    },

    // System/Internal errors (6000s)
    {
        ERROR_MEMORY_ALLOCATION_FAILED,
        FATAL,
        "memory allocation failed",
        "the system ran out of available memory",
        "insufficient memory",
        "try closing other programs or simplifying the code"
    },
    {
        ERROR_SYMBOL_TABLE_CREATION_FAILED,
        FATAL,
        "symbol table creation failed",
        "unable to create or initialize the symbol table",
        "internal symbol table error",
        "restart the compiler or report this issue"
    },
    {
        ERROR_CONTEXT_CREATION_FAILED,
        FATAL,
        "context creation failed",
        "unable to create parsing or execution context",
        "internal context error",
        "restart the compiler or report this issue"
    },
    {
        ERROR_INTERNAL_PARSER_ERROR,
        FATAL,
        "internal parser error",
        "an unexpected error occurred during parsing",
        "compiler bug detected",
        "please report this issue with the problematic code"
    },
    {
        ERROR_INTERNAL_CODE_GENERATOR_ERROR,
        FATAL,
        "internal code generator error",
        "an unexpected error occurred during code generation",
        "compiler bug in code generation",
        "please report this issue with the problematic code"
    },
    {
        ERROR_FILE_NOT_FOUND,
        ERROR,
        "file not found",
        "the specified file could not be located",
        "missing input file",
        "check the file path and ensure the file exists"
    },
    {
        ERROR_FILE_READ_ERROR,
        ERROR,
        "file read error",
        "unable to read from the specified file",
        "I/O error",
        "check file permissions and disk space"
    },
    {
        ERROR_INTERNAL_TYPECHECKER_ERROR,
        ERROR,
        "internal typechecker error",
        "unexpected error occurred during semantic analysis",
        "semantic analysis error",
        "please report this issue with the problematic code"
    },

    // Sentinel
    {
        ERROR_OK,
        ERROR,
        "unknown error",
        "an unspecified error occurred",
        "error details unavailable",
        "check the compiler output"
    }
};


const size_t errorDatabaseCount = sizeof(errorDatabase) / sizeof(errorDatabase[0]);
