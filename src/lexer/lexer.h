/**
* @file lexer.h
 * @brief Lexical analysis interface for the C compiler.
 *
 * Defines token types, data structures, and function prototypes for
 * the two-phase lexical analysis process: splitting and tokenization.
 * Supports comprehensive token recognition including keywords, operators,
 * literals, and special symbols.
 */

#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

// --- Defines ---
#define ASSIGNEMENT "="
#define PUNCTUATION ";"
#define INT_DEFINITION "int"
#define STRING_DEFINITION "string"
#define FLOAT_DEFINITION "float"
#define BOOL_DEFINITION "bool"
#define TRUE_STATEMENT "true"
#define FALSE_STATEMENT "false"
#define QUOTES "\""
#define SUM_OPERATOR "+"
#define SUB_OPERATOR "-"
#define MULTIPLY_OPERATOR "*"
#define DIVIDE_OPERATOR "/"
#define MODULUS_OPERATOR "%"
#define PLUS_ASSIGN "+="
#define SUB_ASSIGN "-="
#define MULTIPLY_ASSIGN "*="
#define DIVIDE_ASSIGN "/="
#define INCREMENT_OPERATOR "++"
#define DECREMENT_OPERATOR "--"
#define LOGICAL_AND "&&"
#define LOGICAL_OR "||"
#define LOGICAL_NOT "!"
#define EQUAL_OPERATOR "=="
#define NOT_EQUAL_OPERATOR "!="
#define LESS_THAN_OPERATOR "<"
#define GREATER_THAN_OPERATOR ">"
#define LESS_EQUAL_OPERATOR "<="
#define GREATER_EQUAL_OPERATOR ">="
#define RIGHT_BRACE "}"
#define LEFT_BRACE "{"
#define LEFT_PAREN "("
#define RIGHT_PAREN ")"
#define COMMA ","

/**
 * @brief Enumeration of all possible token types in the language.
 *
 */
typedef enum {
    TokenAssignement,
    TokenLiteral,
    TokenIntDefinition,
    TokenStringDefinition,
    TokenFloatDefinition,
    TokenBoolDefinition,
    TokenPunctuation,
    TokenQuotes,
    TokenTrue,
    TokenFalse,
    TokenSum,
    TokenSub,
    TokenMult,
    TokenDiv,
    TokenMod,
    TokenIncrement,
    TokenDecrement,
    TokenAnd,
    TokenOr,
    TokenNot,
    TokenString,
    TokenPlusAssign,
    TokenSubAssign,
    TokenMultAssign,
    TokenDivAssign,
    TokenEqual,
    TokenNotEqual,
    TokenLess,
    TokenGreater,
    TokenLessEqual,
    TokenGreaterEqual,
    TokenLeftBrace,
    TokenRightBrace,
    TokenLeftParen,
    TokenRightParen,
    TokenComma,
    TokenNULL
} TokenType;

/**
 * @brief Structure mapping token strings to their corresponding types.
 *
 * Used in static lookup tables to convert string literals to token types.
 * The mapping array must be terminated with a NULL entry.
 */
typedef struct {
    char *value;
    TokenType type;
} TokenMap;

/**
 * @brief Static lookup table mapping token strings to types.
 *
 * Used by findTokenType() to identify token types during lexical analysis.
 * Order matters for multi-character operators - longer operators should
 * appear before their shorter prefixes (e.g., "+=" before "+").
 *
 * @note The array MUST be terminated with a {NULL, TokenLiteral} entry.
 */
static const TokenMap tokenMapping[] = {
    {INT_DEFINITION, TokenIntDefinition},
    {STRING_DEFINITION, TokenStringDefinition},
    {FLOAT_DEFINITION, TokenFloatDefinition},
    {BOOL_DEFINITION, TokenBoolDefinition},
    {TRUE_STATEMENT, TokenTrue},
    {FALSE_STATEMENT, TokenFalse},
    {ASSIGNEMENT, TokenAssignement},
    {PUNCTUATION, TokenPunctuation},
    {QUOTES, TokenQuotes},
    {SUM_OPERATOR, TokenSum},
    {SUB_OPERATOR, TokenSub},
    {MULTIPLY_OPERATOR, TokenMult},
    {DIVIDE_OPERATOR, TokenDiv},
    {MODULUS_OPERATOR, TokenMod},
    {PLUS_ASSIGN, TokenPlusAssign},
    {SUB_ASSIGN, TokenSubAssign},
    {MULTIPLY_ASSIGN, TokenMultAssign},
    {DIVIDE_ASSIGN, TokenDivAssign},
    {INCREMENT_OPERATOR, TokenIncrement},
    {DECREMENT_OPERATOR, TokenDecrement},
    {LOGICAL_AND, TokenAnd},
    {LOGICAL_OR, TokenOr},
    {LOGICAL_NOT, TokenNot},
    {EQUAL_OPERATOR, TokenEqual},
    {NOT_EQUAL_OPERATOR, TokenNotEqual},
    {LESS_THAN_OPERATOR, TokenLess},
    {GREATER_THAN_OPERATOR, TokenGreater},
    {LESS_EQUAL_OPERATOR, TokenLessEqual},
    {GREATER_EQUAL_OPERATOR, TokenGreaterEqual},
    {LEFT_BRACE, TokenLeftBrace},
    {RIGHT_BRACE, TokenRightBrace},
    {LEFT_PAREN, TokenLeftParen},
    {RIGHT_PAREN, TokenRightParen},
    {COMMA, TokenComma},
    {NULL, TokenLiteral} // This MUST be the last entry
};

/**
 * @brief Raw token structure from the splitting phase.
 *
 * Contains the basic token information extracted during the first phase
 * of lexical analysis. Position information is preserved for error reporting.
 */
typedef struct {
    char *value;
    int line;
    int column;
} InputToken;

/**
 * @brief Container for the array of raw tokens from splitting.
 *
 * Manages a dynamic array of InputToken structures with capacity tracking
 * for efficient memory usage and reallocation.
 */
struct Input {
    InputToken *tokens;
    int n;
    int capacity;
};

/**
 * @brief Final token structure for parser consumption.
 *
 * Represents a fully processed token with type information, ready for
 * parsing. Forms a linked list for efficient sequential access by the parser.
 */
typedef struct Input *Input;

// --- Final Token Struct ---
struct Token {
    TokenType type;
    char *value;
    int line;
    int column;
    struct Token *next;
};

typedef struct Token *Token;

// --- Function Prototypes ---
Input splitter(const char *input);

Token tokenization(Input in);

TokenType findTokenType(const char *val);

void freeInput(Input in);

void freeTokenList(Token token);

#endif //LEXER_H
