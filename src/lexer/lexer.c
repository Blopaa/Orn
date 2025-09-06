/**
* @file lexer.c
 * @brief Implementation of lexical analysis functions for the C compiler.
 *
 * Provides a two-phase lexical analysis system:
 * 1. Splitting: Raw character-level parsing with position tracking
 * 2. Tokenization: Type classification and linked list construction
 *
 * Handles comprehensive token recognition including multi-character operators,
 * string literals, numeric literals, comments, and proper error reporting.
 */

#include "lexer.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Finds the token type for a given string value.
 *
 * Performs linear search through the static tokenMapping array to classify
 * the input string. This function is the core of token type determination.
 *
 * @param val Null-terminated string to classify
 * @return TokenType corresponding to the string, or TokenLiteral if not found
 *
 * @note Time complexity is O(n) where n is the number of entries in tokenMapping.
 *       For better performance with many tokens, consider hash table lookup.
 */
TokenType findTokenType(const char *val) {
    for (int i = 0; tokenMapping[i].value != NULL; i++) {
        if (strcmp(tokenMapping[i].value, val) == 0) {
            return tokenMapping[i].type;
        }
    }
    return TokenLiteral;
}

/**
 * @brief Scans input string and splits it into raw tokens with position info.
 *
 * Performs the first phase of lexical analysis by character-level scanning.
 * Handles whitespace, comments, and complex token boundary detection.
 * Maintains line and column information for error reporting.
 *
 * Key features:
 * - Dynamic array allocation with capacity management
 * - Accurate position tracking (line/column)
 * - Comment removal (single-line // comments)
 * - String literal parsing with quote handling
 * - Multi-character operator recognition
 * - Number parsing (integers and floats)
 * - Identifier and keyword recognition
 *
 * @param input Source code string to tokenize
 * @return Input structure containing dynamic array of raw tokens
 *
 * @note The returned Input structure owns all allocated memory and must be
 *       passed to tokenization() or freed with freeInput().
 */
Input splitter(const char *input) {
    Input in = malloc(sizeof(struct Input));
    in->capacity = 64; // Initial capacity
    in->n = 0;
    in->tokens = malloc(in->capacity * sizeof(InputToken));

    int i = 0;
    int line = 1;
    int start_of_line = 0;

    while (input[i] != '\0') {
        if (isspace(input[i])) {
            if (input[i] == '\n') {
                line++;
                start_of_line = i + 1;
            }
            i++;
            continue;
        }

        if (input[i] == '/' && input[i + 1] == '/') {
            while (input[i] != '\n' && input[i] != '\0') {
                i++;
            }
            continue;
        }

        int token_start_pos = i;
        int col = (token_start_pos - start_of_line) + 1;
        int token_len = 0;

        // Logic to determine token length
        if (strchr("(){}[];,", input[i])) {
            token_len = 1;
        } else if (strchr("=!<>", input[i])) {
            if (input[i+1] == '=') token_len = 2;
            else token_len = 1;
        } else if (strchr("+-*/%&|", input[i])) {
            if ((input[i] == '+' && (input[i+1] == '+' || input[i+1] == '=')) ||
                (input[i] == '-' && (input[i+1] == '-' || input[i+1] == '=')) ||
                (input[i] == '&' && input[i+1] == '&') ||
                (input[i] == '|' && input[i+1] == '|') ||
                (input[i] == '*' && input[i+1] == '=') ||
                (input[i] == '/' && input[i+1] == '=')) {
                token_len = 2;
            } else {
                token_len = 1;
            }
        } else if (input[i] == '\"') {
            // String literal parsing - FIXED
            int start = i;
            i++; // Skip opening quote
            while (input[i] != '\"' && input[i] != '\0') i++;
            if (input[i] == '\"') i++; // Skip closing quote
            token_len = i - start;
            // Don't reset i here - keep the new position
        } else if (isdigit(input[i]) || (input[i] == '.' && isdigit(input[i+1]))) {
            int start = i;
            if (input[i] != '.') while (isdigit(input[i])) i++;
            if (input[i] == '.') {
                i++;
                while (isdigit(input[i])) i++;
            }
            token_len = i - start;
            i = start; // Reset for this case only
        } else if (isalpha(input[i]) || input[i] == '_') {
            int start = i;
            while (isalnum(input[i]) || input[i] == '_') i++;
            token_len = i - start;
            i = start; // Reset for this case only
        }

        if (token_len > 0) {
            if (in->n >= in->capacity) {
                in->capacity *= 2;
                in->tokens = realloc(in->tokens, in->capacity * sizeof(InputToken));
            }

            InputToken *raw_token = &in->tokens[in->n++];
            raw_token->value = malloc(token_len + 1);
            strncpy(raw_token->value, &input[token_start_pos], token_len);
            raw_token->value[token_len] = '\0';
            raw_token->line = line;
            raw_token->column = col;

            // Only advance i if we reset it above
            if (input[token_start_pos] != '\"') {
                i += token_len;
            }
            // For strings, i is already at the correct position
        } else {
            if (input[i] != '\0') i++; // Skip unrecognized character
        }
    }
    return in;
}

/**
 * @brief Converts raw tokens to typed tokens in a linked list structure.
 *
 * Performs the second phase of lexical analysis by taking the raw tokens
 * from splitter() and classifying them into specific token types using
 * the tokenMapping lookup table. Creates a linked list suitable for
 * sequential parser consumption.
 *
 * Process:
 * 1. Creates dummy head node for easier list manipulation
 * 2. Transfers string ownership from InputToken to Token
 * 3. Classifies each token using findTokenType()
 * 4. Preserves position information for error reporting
 * 5. Frees the intermediate Input structure
 *
 * @param in Input structure from splitter() (ownership transferred)
 * @return Token linked list with dummy head node
 *
 * @note The input parameter 'in' is consumed and freed by this function.
 *       The returned Token list owns all string memory and should be
 *       freed with freeTokenList() when no longer needed.
 */
Token tokenization(Input in) {
    if (in == NULL) return NULL;

    Token head = malloc(sizeof(struct Token)); // Dummy head node
    head->next = NULL;
    head->value = NULL;
    Token current_token_node = head;

    for (int i = 0; i < in->n; i++) {
        InputToken *raw_token = &in->tokens[i];
        
        Token new_token = malloc(sizeof(struct Token));
        
        new_token->value = raw_token->value; // Transfer ownership of the string
        new_token->type = findTokenType(new_token->value);
        new_token->line = raw_token->line;
        new_token->column = raw_token->column;
        new_token->next = NULL;

        current_token_node->next = new_token;
        current_token_node = new_token;
    }

    // The strings now belong to the Token list, so we can free the intermediate containers.
    free(in->tokens);
    free(in);

    return head;
}

/**
 * @brief Frees an Input structure without converting to tokens.
 *
 * Use this function only if you call splitter() but decide not to proceed
 * with tokenization(). This prevents memory leaks in error conditions or
 * when tokenization is not needed.
 *
 * @param in Input structure to free (can be NULL)
 *
 * @warning Do not call this if you have already called tokenization(),
 *          as tokenization() consumes and frees the Input structure.
 */
void freeInput(Input in) {
    if (in == NULL) return;
    for (int i = 0; i < in->n; i++) {
        free(in->tokens[i].value);
    }
    free(in->tokens);
    free(in);
}

/**
 * @brief Frees a complete Token linked list and all associated memory.
 *
 * Traverses the entire linked list, freeing each Token node and its
 * associated string memory. Safe to call on NULL pointer or empty list.
 *
 * @param token Head of Token linked list (including dummy head node)
 *
 * @note This function properly handles the dummy head node created by
 *       tokenization() and frees all memory associated with the token list.
 */
void freeTokenList(Token token) {
    Token current = token;
    while (current != NULL) {
        Token next = current->next;
        if (current->value != NULL) {
            free(current->value);
        }
        free(current);
        current = next;
    }
}
