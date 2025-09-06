//
// Created by pablo on 13/03/2025.
//

#include "lexer.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// finds the type of the token by keys if didnt found a type for the key its considered a literal
TokenType findTokenType(const char *val) {
    for (int i = 0; tokenMapping[i].value != NULL; i++) {
        if (strcmp(tokenMapping[i].value, val) == 0) {
            return tokenMapping[i].type;
        }
    }
    return TokenLiteral;
}

/**
 * Scans the input string and splits it into a list of raw tokens,
 * capturing the value, line, and column for each.
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
}/**
 * Converts the intermediate list of raw tokens from the splitter into
 * the final linked list of typed Tokens for the parser.
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
 * Frees the intermediate Input struct. Use this only if you call splitter()
 * but decide NOT to call tokenization().
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
 * Frees the final linked list of Tokens.
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
