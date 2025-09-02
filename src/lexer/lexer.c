//
// Created by pablo on 13/03/2025.
//

#include "lexer.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "../errorHandling/errorHandling.h"

// returns 1 if char evaluated its an specialChar saved on SpecialCharMap or 0 if its not
int isSpecialChar(char c) {
    for (int i = 0; SpecialCharMap[i] != NULL; i++) {
        if (c == SpecialCharMap[i][0]) return 1;
    }
    return 0;
}

/*
 * the way of splitting works by saving the current position of were we are working, work to the desire spot, and then
 * get what would be the token lenght and splitting that content
 * example: strncpy(token, input + j, tokenLength);
 */
Input splitter(const char *input) {
    Input in = malloc(sizeof(struct Input));
    int tokenCount = 0;
    for (int i = 0; input[i] != '\0'; i++) {
        tokenCount++;
    }
    in->n = 0;
    char **tokens = malloc((tokenCount + 1) * sizeof(char *));
    in->input = tokens;

    int i = 0;

    while (input[i] != '\0') {
        while ((input[i] == '\n' || isspace(input[i])) && input[i] != '\0') {
            i++;
        }

        // takes care of comments "//"
        if (input[i] == '/' && input[i + 1] == '/') {
            while (input[i] != '\n' && input[i] != '\0') {
                i++;
            }
            continue;
        }

        if (input[i] == '\"') {
            int j = i;
            i++;

            while (input[i] != '\"' && input[i] != '\0') {
                i++;
            }

            if (input[i] != '\0') i++;
            int strLength = i - j;
            char *token = malloc((strLength + 1) * sizeof(char));
            strncpy(token, input + j, strLength);
            token[strLength] = '\0';
            tokens[in->n++] = token;

            // negative number handling
            /*
             * works by assumption of its context:
             * in order to not mix sub from a -5 we have to check previous tokens like = or operations
             */
        } else if (input[i] == '-') {
            if (input[i + 1] == '=') {
                // Handle -= compound operator
                char *token = malloc(3 * sizeof(char));
                token[0] = input[i];
                token[1] = input[i + 1];
                token[2] = '\0';
                tokens[in->n++] = token;
                i += 2;
            } else if (input[i + 1] == '-') {
                // Handle -- decrement operator
                char *token = malloc(3 * sizeof(char));
                token[0] = input[i];
                token[1] = input[i + 1];
                token[2] = '\0';
                tokens[in->n++] = token;
                i += 2;
            } else {
                int isNegativeNumber = 0;
                if (input[i + 1] != '\0' && (isdigit(input[i + 1]))) {
                    if (in->n == 0) isNegativeNumber = 1;
                    else {
                        char *lastToken = tokens[in->n - 1];
                        if (strcmp(lastToken, "=") == 0 ||
                            strcmp(lastToken, "+") == 0 ||
                            strcmp(lastToken, "-") == 0 ||
                            strcmp(lastToken, "*") == 0 ||
                            strcmp(lastToken, "/") == 0) {
                            isNegativeNumber = 1;
                        }
                    }
                }
                if (isNegativeNumber) {
                    int j = i;
                    i++;
                    while (!isspace(input[i]) && input[i] != '\n' && !isSpecialChar(input[i]) && input[i] != '\0') {
                        i++;
                    }
                    int tokenLength = i - j;
                    if (tokenLength > 1) {
                        // has to be more than just a - sign
                        char *token = malloc((tokenLength + 1) * sizeof(char));
                        strncpy(token, input + j, tokenLength);
                        token[tokenLength] = '\0';
                        tokens[in->n++] = token;
                    }
                } else {
                    // NOT a negative number, treat as subtraction operator
                    char *token = malloc(2 * sizeof(char));
                    token[0] = input[i];
                    token[1] = '\0';
                    tokens[in->n++] = token;
                    i++;
                }
            }
        } else if ((input[i] == '+' || input[i] == '*' || input[i] == '/' || input[i] == '-') && (
                       input[i + 1] == '=' || input[i + 1] == '+' || input[i + 1] == '-')) {
            // Handle compound operators for +=, *=, /=
            char *token = malloc(3 * sizeof(char));
            token[0] = input[i];
            token[1] = input[i + 1];
            token[2] = '\0';
            tokens[in->n++] = token;
            i += 2;
        } else if (isSpecialChar(input[i])) {
            char *token = malloc(2 * sizeof(char));
            token[0] = input[i];
            token[1] = '\0';
            tokens[in->n++] = token;
            i++;
        } else {
            int j = i;
            while (!isspace(input[i]) && input[i] != '\n' && !isSpecialChar(input[i]) && input[i] != '\0') {
                i++;
            }
            int tokenLength = i - j;
            // creates tokens if there is content (only comment code case)
            if (tokenLength > 0) {
                char *token = malloc((tokenLength + 1) * sizeof(char));
                strncpy(token, input + j, tokenLength);
                token[tokenLength] = '\0';
                tokens[in->n++] = token;
            }
        }
    }

    tokens[in->n] = NULL;
    return in;
}

//creates a new token given a type and its value
Token createToken(char *val, TokenType t) {
    Token tk = malloc(sizeof(struct Token));
    tk->type = t;
    tk->value = val;
    return tk;
}

// finds the type of the token by keys if didnt found a type for the key its considered a literal
TokenType findTokenType(const char *val) {
    for (int i = 0; tokenMapping[i].value != NULL; i++) {
        if (strcmp(tokenMapping[i].value, val) == 0) {
            return tokenMapping[i].type;
        }
    }
    return TokenLiteral;
}

Token tokenization(Input in) {
    struct Token *const head = malloc(sizeof(struct Token));
    Token crrnt = head;
    for (int i = 0; i < in->n; i++) {
        crrnt->next = malloc(sizeof(struct Token));
        crrnt = crrnt->next;

        crrnt->type = findTokenType(in->input[i]);
        crrnt->value = in->input[i];
    }
    crrnt->next = NULL;
    return head;
}
