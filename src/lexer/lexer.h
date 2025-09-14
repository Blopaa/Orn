#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Enumeration of all possible token types in the language.
 *
 */
typedef enum {
    TK_ASSIGN,
    TK_LIT,
    TK_INT,
    TK_STRING,
    TK_FLOAT,
    TK_BOOL,
    TK_FN,
    TK_VOID,
    TK_RETURN,
    TK_ARROW,
    TK_SEMI,
    TK_QUOTE,
    TK_WHILE,
    TK_TRUE,
    TK_FALSE,
    TK_PLUS,
    TK_MINUS,
    TK_STAR,
    TK_SLASH,
    TK_MOD,
    TK_INCR,
    TK_DECR,
    TK_AND,
    TK_OR,
    TK_NOT,
    TK_STR,
	TK_NUM,
    TK_PLUS_ASSIGN,
    TK_MINUS_ASSIGN,
    TK_STAR_ASSIGN,
    TK_SLASH_ASSIGN,
    TK_EQ,
    TK_NOT_EQ,
    TK_LESS,
    TK_GREATER,
    TK_LESS_EQ,
    TK_GREATER_EQ,
    TK_LBRACE,
    TK_RBRACE,
    TK_LPAREN,
    TK_RPAREN,
    TK_COMMA,
    TK_QUESTION,
    TK_COLON,
	TK_EOF,
	TK_INVALID,
    TK_NULL
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
}TokenList;

typedef struct {
	const char *src;
	const char *cur;
	size_t line;
	size_t col;
	size_t line_start;
	TokenList *list;
} Lexer;

TokenList* lex(const char *input);
void freeTokens(TokenList *list);
const char* tokenName(TokenType type);
char *tokenToString(const Token *tok);

#endif //LEXER_H
