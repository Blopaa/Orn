#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Enumeration of all possible token types in the language.
 *
 */
typedef enum {
	// Keywords
	TK_STRUCT,
	TK_FN,
	TK_VOID,
	TK_RETURN,
	TK_WHILE,
	TK_TRUE,
	TK_FALSE,
	TK_AS,

	// Data types
	TK_INT,
	TK_STRING,
	TK_FLOAT,
	TK_BOOL,
	TK_DOUBLE,

	// Literals
	TK_LIT,
	TK_STR,
	TK_NUM,

	// Arithmetic operators
	TK_PLUS,
	TK_MINUS,
	TK_STAR,
	TK_SLASH,
	TK_MOD,
	TK_INCR,
	TK_DECR,

	// Assignment operators
	TK_ASSIGN,
	TK_PLUS_ASSIGN,
	TK_MINUS_ASSIGN,
	TK_STAR_ASSIGN,
	TK_SLASH_ASSIGN,

	// Comparison operators
	TK_EQ,
	TK_NOT_EQ,
	TK_LESS,
	TK_GREATER,
	TK_LESS_EQ,
	TK_GREATER_EQ,

	// Logical operators
	TK_AND,
	TK_OR,
	TK_NOT,

	// Delimiters
	TK_LBRACE,
	TK_RBRACE,
	TK_LPAREN,
	TK_RPAREN,
	TK_SEMI,
	TK_COMMA,
	TK_QUOTE,
	TK_ARROW,
	TK_QUESTION,
	TK_COLON,
	TK_DOT,

	// Special tokens
	TK_NULL,
	TK_EOF,
	TK_INVALID
} TokenType;

typedef struct Token {
    TokenType type;
    const char *start;
	uint16_t length;
    uint16_t line;
    uint16_t column;
} Token;

typedef struct TokenList {
	Token *tokens;
	size_t count;
	size_t capacity;
	char * buffer;
	const char *filename;
}TokenList;

typedef struct {
	const char *src;
	const char *cur;
	size_t line;
	size_t col;
	size_t line_start;
	TokenList *list;
} Lexer;

TokenList* lex(const char *input, const char *filename);
void freeTokens(TokenList *list);
const char* tokenName(TokenType type);
char *extractSourceLineForToken(TokenList *list, Token *token);

#endif //LEXER_H
