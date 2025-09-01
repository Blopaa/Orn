//
// Created by pablo on 20/05/2025.
//

#include "parser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../errorHandling/errorHandling.h"
#include "../lexer/lexer.h"

// returns the type of the variable declaration based on the type of the token given string | int
// will return null_NODE as sentinel type when its not a type of variableDec
NodeTypes getDecType(TokenType type) {
    for (int i = 0; TypeDefs[i].TkType != TokenNULL; i++) {
        if (TypeDefs[i].TkType == type) return TypeDefs[i].type;
    }
    return null_NODE;
}

// checks if its compoundAssign =+, -=...
int isCompoundAssignType(TokenType type) {
    return (type == TokenPlusAssign || type == TokenSubAssign || type == TokenMultAssign || type == TokenDivAssign);
}

//  returns the respective Node for each type, if it doesn't match returns null_NODE
NodeTypes getCompoundNodeType(TokenType tokenType) {
    switch (tokenType) {
        case TokenPlusAssign: return COMPOUND_ADD_ASSIGN;
        case TokenSubAssign: return COMPOUND_SUB_ASSIGN;
        case TokenMultAssign: return COMPOUND_MUL_ASSIGN;
        case TokenDivAssign: return COMPOUND_DIV_ASSIGN;
        default: return null_NODE;
    }
}

//creates a new node given a value and a type
ASTNode createNode(char *val, NodeTypes type) {
    ASTNode node = malloc(sizeof(struct ASTNode));
    node->value = val ? strdup(val) : NULL;;
    node->NodeType = type;
    node->brothers = NULL;
    node->children = NULL;
    return node;
}

// checks if its a float, also works for ints, as a float it can have int signature so it would depend on set type
int isFloatLit(char *val) {
    if (val == NULL) return 0;

    int hasDecPoint = 0;
    int start = 0;

    // Handle negative sign
    if (val[0] == '-') {
        start = 1;
        // Must have at least one character after the minus sign
        if (val[1] == '\0') return 0;
    }

    for (int i = start; val[i] != '\0'; i++) {
        if (val[i] == '.') {
            hasDecPoint = 1;
        }
    }

    return hasDecPoint;
}

int validateFloatLit(char *val) {
    if (val == NULL) return 0;

    int start = 0;
    // Handle negative sign
    if (val[0] == '-') {
        start = 1;
        if (val[1] == '\0') {
            repError(ERROR_INVALID_FLOAT_NO_DIGITS, val);
            return 0;
        }
    }

    if (strchr(val + start, '.') == NULL) {
        return 0;
    }

    int decimalCount = 0;
    int hasDigits = 0;

    for (int i = start; val[i] != '\0'; i++) {
        if (val[i] == '.') {
            if (decimalCount) {
                repError(ERROR_INVALID_FLOAT_MULTIPLE_DECIMALS, val);
                return 0;
            };
            decimalCount = 1;
        } else if (isdigit(val[i])) {
            hasDigits = 1;
        } else {
            repError(ERROR_INVALID_FLOAT_INVALID_CHAR, val);
            return 0;
        }
    }

    if (!hasDigits) {
        repError(ERROR_INVALID_FLOAT_NO_DIGITS, val);
        return 0;
    }

    return 1;
}

//checks if its a string
int isStringLit(char *val) {
    // ERROR REP SHOULDN'T BE HERE, BUT AS IF SOMETHING HAS QUOTES CANT BE A NUMBER NOR A VARIABLE IT WILL BE TAKEN AS A TRY TO INPUT AN STRING
    // FIX: checker takes anything with a quote and then verifies if it's correct.
    if ((strchr(val, '"') && val[0] != '"') || (strchr(val, '"') && val[strlen(val) - 1] != '"')) {
        repError(ERROR_MISSING_QUOTE, val);
        return 0;
    }
    return val != NULL && val[0] == '"' && val[strlen(val) - 1] == '"';
}

//checks if its an int
int isIntLit(char *val) {
    if (val == NULL) return 0;
    int start = 0;

    // Handle negative sign
    if (val[0] == '-') {
        start = 1;
        // Must have at least one digit after the minus sign
        if (val[1] == '\0') return 0;
    }

    for (int i = start; val[i] != '\0'; i++) {
        if (!isdigit(val[i])) return 0;
    }
    return (val[start] != '\0');
}

//checks if its not a int nor a string
int isVariable(char *val) {
    if (val == NULL) return 0;
    return !(isStringLit(val) && isIntLit(val) && isFloatLit(val));
}

int isValidVariable(char *val) {
    if (val == NULL) return 0;

    // Check if it's just an operator
    if (strcmp(val, "-") == 0 || strcmp(val, "+") == 0 ||
        strcmp(val, "*") == 0 || strcmp(val, "/") == 0 ||
        strcmp(val, "=") == 0 || strcmp(val, ";") == 0) {
        return 0;
    }
    // Check if it starts with a letter or underscore (proper variable naming)
    if (!isalpha(val[0]) && val[0] != '_') {
        return 0;
    }

    return 1;
}

//classifies variable possible values, int, string,....
ASTNode createValNode(char *val, NodeTypes fatherType) {
    ASTNode valNod = NULL;
    if (isStringLit(val)) {
        if (fatherType == INT_VARIABLE_DEFINITION || fatherType == FLOAT_VARIABLE_DEFINITION) {
            ErrorCode errorType = (fatherType == INT_VARIABLE_DEFINITION)
                                      ? ERROR_TYPE_MISMATCH_STRING_TO_INT
                                      : ERROR_TYPE_MISMATCH_STRING_TO_FLOAT;
            repError(errorType, val);
            return NULL;
        }
        valNod = createNode(val, STRING_LIT);
    } else if (isFloatLit(val)) {
        if (!validateFloatLit(val)) {
            return NULL;
        }
        if (fatherType == STRING_VARIABLE_DEFINITION) {
            repError(ERROR_TYPE_MISMATCH_FLOAT_TO_STRING, val);
            return NULL;
        }
        if (fatherType == INT_VARIABLE_DEFINITION) {
            repError(ERROR_TYPE_MISMATCH_FLOAT_TO_INT, val);
            return NULL;
        }
        valNod = createNode(val, FLOAT_LIT);
    } else if (isIntLit(val)) {
        if (fatherType == STRING_VARIABLE_DEFINITION) {
            repError(ERROR_TYPE_MISMATCH_INT_TO_STRING, val);
            return NULL;
        }
        valNod = createNode(val, INT_LIT);
    } else if (isValidVariable(val)) {
        valNod = createNode(val, VARIABLE);
    } else {
        repError(ERROR_INVALID_EXPRESSION, val);
        return NULL;
    }
    return valNod;
}

// returns if found respective NodeType for the operator send, otherwise returns null_Node
NodeTypes getOperatorType(char *val) {
    switch (val[0]) {
        case '+': return ADD_OP;
        case '-': return SUB_OP;
        case '*': return MUL_OP;
        case '/': return DIV_OP;
        default: return null_NODE;
    }
}

ASTNode parsePrimaryExp(Token *current, NodeTypes fatherType) {
    if (*current == NULL) return NULL;
    ASTNode node = createValNode((*current)->value, fatherType);
    if (node != NULL) {
        *current = (*current)->next;
    }
    return node;
}

ASTNode parseMulDivExp(Token *current, NodeTypes fatherType) {
    ASTNode left = parsePrimaryExp(current, fatherType);
    if (left == NULL) return NULL;

    while (*current != NULL && ((*current)->type == TokenMult || (*current)->type == TokenDiv)) {
        Token opToken = *current;
        *current = (*current)->next;
        ASTNode right = parsePrimaryExp(current, fatherType);
        if (right == NULL) return left;
        NodeTypes opType = getOperatorType(opToken->value);
        ASTNode opNode = createNode(opToken->value, opType);
        opNode->children = left;
        left->brothers = right;
        left = opNode;
    }
    return left;
}

ASTNode parseAddSubExp(Token *current, NodeTypes fatherType) {
    ASTNode left = parseMulDivExp(current, fatherType);
    if (left == NULL) return NULL;
    while (*current != NULL && ((*current)->type == TokenSum || (*current)->type == TokenSub)) {
        Token opToken = *current;
        *current = (*current)->next;
        ASTNode right = parseMulDivExp(current, fatherType);
        if (right == NULL) {
            return left;
        }

        NodeTypes opType = getOperatorType(opToken->value);
        ASTNode opNode = createNode(opToken->value, opType);
        opNode->children = left;
        left->brothers = right;
        left = opNode;
    }
    return left;
}

//splits arithmetic expressions into two branches of a variable definition
//AST:
//└── INT_VAR_DEF: variableName
//    └── ADD_OP
//        ├── INT_LIT: 2 (value)
//        └── VARIABLE: b (literal)
ASTNode ExpParser(Token *crrnt, NodeTypes fatherType) {
    return parseAddSubExp(crrnt, fatherType);
}

// generates AST
ASTNode ASTGenerator(Token token) {
    ASTNode programNode = createNode("PROGRAM", null_NODE);
    ASTNode crrntStat = NULL; // current statement
    ASTNode ls = NULL; // last statement

    while (token && token->next != NULL) {
        token = token->next;

        NodeTypes type = getDecType(token->type);
        if (type != null_NODE) {
            crrntStat = createNode(NULL, type);
            if (token->next != NULL) {
                token = token->next;
                crrntStat->value = strdup(token->value);
                if (token->next != NULL && token->next->type == TokenAssignement) {
                    token = token->next;
                    if (token->next != NULL) {
                        token = token->next;
                        ASTNode valNod = ExpParser(&token, type);
                        if (valNod) {
                            crrntStat->children = valNod;
                        }
                        // IMPORTANT: Always advance to semicolon or end, even if ExpParser failed
                        while (token && token->next != NULL && token->next->type != TokenPunctuation) {
                            token = token->next;
                        }
                        if (token && token->next != NULL && token->next->type == TokenPunctuation) {
                            token = token->next; // skip ";"
                        }
                    }
                } else {
                    // No assignment, just skip to semicolon
                    while (token && token->next != NULL && token->next->type != TokenPunctuation) {
                        token = token->next;
                    }
                    if (token && token->next != NULL && token->next->type == TokenPunctuation) {
                        token = token->next; // skip ";"
                    }
                }
            }
            if (programNode->children == NULL) {
                programNode->children = crrntStat;
            } else {
                ls->brothers = crrntStat;
            }
            ls = crrntStat;
            //handles variable assignements NOT declarations, dec is top if
        } else if (token->type == TokenLiteral) {
            if (token->next != NULL && token->next->type == TokenAssignement) {
                crrntStat = createNode(strdup(token->value), ASSIGNMENT);
                token = token->next;
                if (token->next != NULL) {
                    token = token->next;
                    ASTNode valNod = ExpParser(&token, type);
                    crrntStat->children = valNod;
                }
                //handles compound assignements like += -= ...
            } else if (token->next != NULL && isCompoundAssignType(token->next->type)) {
                // Compound assignment (x += 5;, x -= 3;, etc.)
                NodeTypes compoundType = getCompoundNodeType(token->next->type);
                crrntStat = createNode(strdup(token->value), compoundType);
                token = token->next; // skip compound operator
                if (token->next != NULL) {
                    token = token->next;
                    ASTNode valNod = ExpParser(&token, null_NODE); // No type constraint for assignments
                    crrntStat->children = valNod;
                }
            }
            // Add to AST if we created a statement
            if (crrntStat != NULL) {
                if (programNode->children == NULL) {
                    programNode->children = crrntStat;
                } else {
                    ls->brothers = crrntStat;
                }
                ls = crrntStat;
            }
        }
    }
    return programNode;
}

void printASTTree(ASTNode node, char *prefix, int isLast) {
    if (node == NULL) return;

    char *nodeTypeStr;
    switch (node->NodeType) {
        case STRING_VARIABLE_DEFINITION: nodeTypeStr = "STRING_VAR_DEF";
            break;
        case INT_VARIABLE_DEFINITION: nodeTypeStr = "INT_VAR_DEF";
            break;
        case FLOAT_VARIABLE_DEFINITION: nodeTypeStr = "FLOAT_VAR_DEF";
            break;
        case STRING_LIT: nodeTypeStr = "STRING_LIT";
            break;
        case INT_LIT: nodeTypeStr = "INT_LIT";
            break;
        case ADD_OP: nodeTypeStr = "ADD_OP";
            break;
        case SUB_OP: nodeTypeStr = "SUB_OP";
            break;
        case MUL_OP: nodeTypeStr = "MUL_OP";
            break;
        case DIV_OP: nodeTypeStr = "DIV_OP";
            break;
        case VARIABLE: nodeTypeStr = "VARIABLE";
            break;
        case FLOAT_LIT: nodeTypeStr = "FLOAT_LIT";
            break;
        case ASSIGNMENT: nodeTypeStr = "ASSIGNMENT";
            break;
        case COMPOUND_ADD_ASSIGN: nodeTypeStr = "COMPOUND_ADD_ASSIGN";
            break;
        case COMPOUND_SUB_ASSIGN: nodeTypeStr = "COMPOUND_SUB_ASSIGN";
            break;
        case COMPOUND_MUL_ASSIGN: nodeTypeStr = "COMPOUND_MULT_ASSIGN";
            break;
        case COMPOUND_DIV_ASSIGN: nodeTypeStr = "COMPOUND_DIV_ASSIGN";
            break;
        default: nodeTypeStr = "UNKNOWN";
            break;
    }

    printf("%s", prefix);
    printf("%s", isLast ? "└── " : "├── ");

    printf("%s", nodeTypeStr);
    if (node->value) {
        printf(": %s", node->value);
    }
    printf("\n");

    char newPrefix[256];
    sprintf(newPrefix, "%s%s", prefix, isLast ? "    " : "│   ");

    if (node->children != NULL) {
        ASTNode child = node->children;
        while (child != NULL) {
            int isLastChild = (child->brothers == NULL);
            printASTTree(child, newPrefix, isLastChild);
            child = child->brothers;
        }
    }
}

void printAST(ASTNode node, int depth) {
    if (node == NULL) return;

    if (depth == 0) {
        printf("AST:\n");

        ASTNode child = node->children;
        while (child != NULL) {
            int isLast = (child->brothers == NULL);
            printASTTree(child, "", isLast);
            child = child->brothers;
        }
    }
}

void freeAST(ASTNode node) {
    if (node == NULL) return;

    freeAST(node->children);
    freeAST(node->brothers);

    if (node->value) {
        free(node->value);
    }
    free(node);
}
