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
    TokenWhileLoop,
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
    TokenQuestion,
    TokenColon,
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
    // Keywords
    {"int", TokenIntDefinition},
    {"string", TokenStringDefinition},
    {"float", TokenFloatDefinition},
    {"bool", TokenBoolDefinition},
    {"true", TokenTrue},
    {"false", TokenFalse},
    {"@", TokenWhileLoop},

    // Multi-character operators (must come before single-char versions)
    {"+=", TokenPlusAssign},
    {"-=", TokenSubAssign},
    {"*=", TokenMultAssign},
    {"/=", TokenDivAssign},
    {"++", TokenIncrement},
    {"--", TokenDecrement},
    {"&&", TokenAnd},
    {"||", TokenOr},
    {"==", TokenEqual},
    {"!=", TokenNotEqual},
    {"<=", TokenLessEqual},
    {">=", TokenGreaterEqual},

    // Single-character operators
    {"=", TokenAssignement},
    {"+", TokenSum},
    {"-", TokenSub},
    {"*", TokenMult},
    {"/", TokenDiv},
    {"%", TokenMod},
    {"!", TokenNot},
    {"<", TokenLess},
    {">", TokenGreater},

    // Delimiters
    {";", TokenPunctuation},
    {"\"", TokenQuotes},
    {"{", TokenLeftBrace},
    {"}", TokenRightBrace},
    {"(", TokenLeftParen},
    {")", TokenRightParen},
    {",", TokenComma},
    {"?", TokenQuestion},
    {":", TokenColon},

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
