#include "lexer.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INITIAL_CAPACITY 256

static TokenType lookUpKeyword(const char * s, size_t len) {
	if (len < 2) return TK_LIT;

	switch (s[0]) {
	case 'b':
		if (len == 4 && memcmp(s, "bool", 4) == 0) return TK_BOOL;
		break;
	case 'f':
		if (len == 2 && s[1] == 'n') return TK_FN;
		if (len == 5) {
			if (memcmp(s, "float", 5) == 0) return TK_FLOAT;
			if (memcmp(s, "false", 5) == 0) return TK_FALSE;
		}
		break;
	case 'i':
		if (len == 3 && memcmp(s, "int", 3) == 0) return TK_INT;
		break;
	case 'r':
		if (len == 6 && memcmp(s, "return", 6) == 0) return TK_RETURN;
		break;
	case 's':
		if (len == 6 && memcmp(s, "string", 6) == 0) return TK_STRING;
		break;
	case 't':
		if (len == 4 && memcmp(s, "true", 4) == 0) return TK_TRUE;
		break;
	case 'v':
		if (len == 4 && memcmp(s, "void", 4) == 0) return TK_VOID;
		break;
	case 'w':
		if (len == 5 && memcmp(s, "while", 5) == 0) return TK_WHILE;
		break;
	}
	return TK_LIT;
}

static void addToken(Lexer *lx, TokenType type, const char * start, size_t len) {
	if (lx->list->count >= lx->list->capacity) {
		lx->list->capacity *= 2;
		lx->list->tokens = realloc(lx->list->tokens, lx->list->capacity * sizeof(Token));
	}
	Token *token = &lx->list->tokens[lx->list->count++];
	token->type = type;
	token->start = start;
	token->length = len;
	token->line = lx->line;
	token->column = (start - lx->src) - lx->line_start + 1;
}

static void skipWhitespace(Lexer *lx) {
	while (*lx->cur) {
		if (*lx->cur == ' ' || *lx->cur == '\t' || *lx->cur == '\r') {
			lx->cur++;
		} else if (*lx->cur == '\n') {
			lx->cur++;
			lx->line++;
			lx->line_start = lx->cur - lx->src;
		} else if (lx->cur[0] == ':' && lx->cur[1] == ':') {
			// Single-line comment
			lx->cur += 2;
			while (*lx->cur && *lx->cur != '\n') lx->cur++;
		} else if (lx->cur[0] == ':' && lx->cur[1] == '|') {
			// Multi-line comment
			lx->cur += 2;
			while (*lx->cur) {
				if (lx->cur[0] == '|' && lx->cur[1] == ':') {
					lx->cur += 2;
					break;
				}
				if (*lx->cur == '\n') {
					lx->line++;
					lx->line_start = lx->cur - lx->src + 1;
				}
				lx->cur++;
			}
		} else {
			break;
		}
	}
}

static void lexString(Lexer *lx) {
	const char *start = lx->cur++;
	while (*lx->cur && *lx->cur != '"') {
		if (*lx->cur == '\\' && lx->cur[1]) lx->cur += 2;
		else lx->cur++;
	}
	if (*lx->cur == '"') lx->cur++;
	addToken(lx, TK_STR, start, lx->cur - start);
}

static void lexNumber(Lexer *lx) {
	const char *start = lx->cur;
	while (isdigit(*lx->cur)) lx->cur++;
	if (*lx->cur == '.' && isdigit(lx->cur[1])) {
		lx->cur++;
		while (isdigit(*lx->cur)) lx->cur++;
	}
	addToken(lx, TK_NUM, start, lx->cur - start);
}

static void lexIdent(Lexer *lx) {
	const char *start = lx->cur;
	while (isalnum(*lx->cur) || *lx->cur == '_') lx->cur++;
	size_t len = lx->cur - start;
	TokenType type = lookUpKeyword(start, len);
	addToken(lx, type, start, lx->cur - start);
}

static void lexOperator(Lexer *lx) {
    const char *start = lx->cur;
    char c = *lx->cur++;
    char next = *lx->cur;

    // Two-character operators
    switch (c) {
        case '+':
            if (next == '=') { lx->cur++; addToken(lx, TK_PLUS_ASSIGN, start, 2); return; }
            if (next == '+') { lx->cur++; addToken(lx, TK_INCR, start, 2); return; }
            addToken(lx, TK_PLUS, start, 1); return;
        case '-':
            if (next == '=') { lx->cur++; addToken(lx, TK_MINUS_ASSIGN, start, 2); return; }
            if (next == '-') { lx->cur++; addToken(lx, TK_DECR, start, 2); return; }
            if (next == '>') { lx->cur++; addToken(lx, TK_ARROW, start, 2); return; }
            addToken(lx, TK_MINUS, start, 1); return;
        case '*':
            if (next == '=') { lx->cur++; addToken(lx, TK_STAR_ASSIGN, start, 2); return; }
            addToken(lx, TK_STAR, start, 1); return;  // Note: TK_STAR not TK_START
        case '/':
            if (next == '=') { lx->cur++; addToken(lx, TK_SLASH_ASSIGN, start, 2); return; }
            addToken(lx, TK_SLASH, start, 1); return;
        case '=':
            if (next == '=') { lx->cur++; addToken(lx, TK_EQ, start, 2); return; }
            addToken(lx, TK_ASSIGN, start, 1); return;
        case '!':
            if (next == '=') { lx->cur++; addToken(lx, TK_NOT_EQ, start, 2); return; }
            addToken(lx, TK_NOT, start, 1); return;
        case '<':
            if (next == '=') { lx->cur++; addToken(lx, TK_LESS_EQ, start, 2); return; }
            addToken(lx, TK_LESS, start, 1); return;
        case '>':
            if (next == '=') { lx->cur++; addToken(lx, TK_GREATER_EQ, start, 2); return; }
            addToken(lx, TK_GREATER, start, 1); return;
        case '&':
            if (next == '&') { lx->cur++; addToken(lx, TK_AND, start, 2); return; }
            addToken(lx, TK_INVALID, start, 1); return;
        case '|':
            if (next == '|') { lx->cur++; addToken(lx, TK_OR, start, 2); return; }
            addToken(lx, TK_INVALID, start, 1); return;
        case '%':
            addToken(lx, TK_MOD, start, 1); return;
        case ';':
            addToken(lx, TK_SEMI, start, 1); return;
        case '{':
            addToken(lx, TK_LBRACE, start, 1); return;
        case '}':
            addToken(lx, TK_RBRACE, start, 1); return;
        case '(':
            addToken(lx, TK_LPAREN, start, 1); return;
        case ')':
            addToken(lx, TK_RPAREN, start, 1); return;
        case ',':
            addToken(lx, TK_COMMA, start, 1); return;
        case '?':
            addToken(lx, TK_QUESTION, start, 1); return;
        case ':':
            addToken(lx, TK_COLON, start, 1); return;
        default:
            addToken(lx, TK_INVALID, start, 1); return;
    }
}

TokenList* lex(const char *input) {
	TokenList *list = malloc(sizeof(TokenList));
	list->capacity = INITIAL_CAPACITY;
	list->count = 0;
	list->tokens = malloc(list->capacity * sizeof(Token));
	list->buffer = strdup(input);  // Keep a copy for token references

	Lexer lx = {
		.src = list->buffer,
		.cur = list->buffer,
		.line = 1,
		.col = 1,
		.line_start = 0,
		.list = list
	};

	while (*lx.cur) {
		skipWhitespace(&lx);
		if (!*lx.cur) break;

		if (*lx.cur == '"') {
			lexString(&lx);
		} else if (isdigit(*lx.cur) || (*lx.cur == '.' && isdigit(lx.cur[1]))) {
			lexNumber(&lx);
		} else if (isalpha(*lx.cur) || *lx.cur == '_') {
			lexIdent(&lx);
		} else {
			lexOperator(&lx);
		}
	}

	addToken(&lx, TK_EOF, lx.cur, 0);
	return list;
}

void freeTokens(TokenList *list) {
	if (!list) return;
	free(list->tokens);
	free(list->buffer);
	free(list);
}

const char* tokenName(TokenType type) {
	static const char *names[] = {
		"ASSIGN", "LIT", "INT", "STRING", "FLOAT", "BOOL", "FN", "VOID", "RETURN", "ARROW",
		"SEMI", "QUOTE", "WHILE", "TRUE", "FALSE", "PLUS", "MINUS", "STAR", "SLASH", "MOD",
		"INCR", "DECR", "AND", "OR", "NOT", "STR", "PLUS_ASSIGN", "MINUS_ASSIGN",
		"STAR_ASSIGN", "SLASH_ASSIGN", "EQ", "NOT_EQ", "LESS", "GREATER", "LESS_EQ",
		"GREATER_EQ", "LBRACE", "RBRACE", "LPAREN", "RPAREN", "COMMA", "QUESTION",
		"COLON", "NULL", "NUM", "EOF", "INVALID"
	};
	return (type < sizeof(names)/sizeof(names[0])) ? names[type] : "UNKNOWN";
}

char *tokenToString(const Token *tok) {
	if (!tok || !tok->start) return strdup("");
	char *buf = malloc(tok->length + 1);
	if (!buf) return NULL;
	memcpy(buf, tok->start, tok->length);
	buf[tok->length] = '\0';
	return buf;
}