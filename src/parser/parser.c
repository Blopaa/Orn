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

ASTNode parseStatement(Token *current); // Forward declaration

// --- HELPER FUNCTIONS ---
NodeTypes getDecType(TokenType type) {
    for (int i = 0; TypeDefs[i].TkType != TokenNULL; i++) {
        if (TypeDefs[i].TkType == type) return TypeDefs[i].type;
    }
    return null_NODE;
}

ASTNode createNode(Token token, NodeTypes type) {
    ASTNode node = malloc(sizeof(struct ASTNode));
    if (node == NULL) return NULL;

    node->value = (token && token->value) ? strdup(token->value) : NULL;
    node->NodeType = type;

    if (token) {
        node->line = token->line;
        node->column = token->column;
    } else {
        node->line = 0;
        node->column = 0;
    }
    
    node->brothers = NULL;
    node->children = NULL;
    return node;
}

// --- VALIDATION FUNCTIONS ---
int isFloatLit(char *val) {
    if (val == NULL) return 0;
    int start = (val[0] == '-') ? 1 : 0;
    if (val[start] == '\0') return 0;
    for (int i = start; val[i] != '\0'; i++) {
        if (val[i] == '.') return 1;
    }
    return 0;
}

int isValidVariable(char *val) {
    if (val == NULL || (!isalpha(val[0]) && val[0] != '_')) {
        return 0;
    }
    for (int i = 1; val[i] != '\0'; i++) {
        if (!isalnum(val[i]) && val[i] != '_') {
            return 0;
        }
    }
    return 1;
}

int isIntLit(char *val) {
    if (val == NULL) return 0;
    int start = (val[0] == '-') ? 1 : 0;
    if (val[start] == '\0') return 0;
    for (int i = start; val[i] != '\0'; i++) {
        if (!isdigit(val[i])) return 0;
    }
    return 1;
}

int isValidStringLit(char *val) {
    if (val == NULL) return 0;
    size_t len = strlen(val);
    return (len >= 2 && val[0] == '"' && val[len - 1] == '"');
}

ASTNode createValNode(Token current_token, NodeTypes fatherType) {
    (void)fatherType;
    if (current_token == NULL) return NULL;
    char *val = current_token->value;
    if (isValidStringLit(val)) return createNode(current_token, STRING_LIT);
    if (isFloatLit(val)) return createNode(current_token, FLOAT_LIT);
    if (isIntLit(val)) return createNode(current_token, INT_LIT);
    if (strcmp(val, "true") == 0 || strcmp(val, "false") == 0) return createNode(current_token, BOOL_LIT);
    if (isValidVariable(val)) return createNode(current_token, VARIABLE);
    repError(ERROR_INVALID_EXPRESSION, val);
    return NULL;
}

const OperatorInfo *getOperatorInfo(TokenType type) {
    for (int i = 0; operators[i].token != TokenNULL; i++) {
        if (operators[i].token == type) return &operators[i];
    }
    return NULL;
}

// --- PARSER IMPLEMENTATION ---
ASTNode parsePrimaryExp(Token *current, NodeTypes fatherType) {
    if (current == NULL || *current == NULL) return NULL;
    ASTNode node = createValNode(*current, fatherType);
    if (node != NULL) {
        *current = (*current)->next;
    }
    return node;
}

ASTNode parseUnary(Token *current, NodeTypes fatherType) {
    if (current == NULL || *current == NULL) return NULL;
    if ((*current)->type == TokenSub || (*current)->type == TokenNot || 
        (*current)->type == TokenIncrement || (*current)->type == TokenDecrement) {
        Token opToken = *current;
        *current = (*current)->next;
        ASTNode operand = parseUnary(current, fatherType);
        if (operand == NULL) return NULL;
        NodeTypes opType;
        if(opToken->type == TokenSub) opType = UNARY_MINUS_OP;
        else if (opToken->type == TokenNot) opType = LOGIC_NOT;
        else if (opToken->type == TokenIncrement) opType = PRE_INCREMENT;
        else opType = PRE_DECREMENT;
        ASTNode opNode = createNode(opToken, opType);
        opNode->children = operand;
        return opNode;
    }
    ASTNode node = parsePrimaryExp(current, fatherType);
    if (node != NULL && *current != NULL && ((*current)->type == TokenIncrement || (*current)->type == TokenDecrement)) {
        Token opToken = *current;
        *current = (*current)->next;
        NodeTypes opType = (opToken->type == TokenIncrement) ? POST_INCREMENT : POST_DECREMENT;
        ASTNode opNode = createNode(opToken, opType);
        opNode->children = node;
        return opNode;
    }
    return node;
}

ASTNode parseExpression(Token *current, NodeTypes fatherType, Precedence minPrec) {
    if (current == NULL || *current == NULL) return NULL;
    ASTNode left = parseUnary(current, fatherType);
    if (left == NULL) return NULL;
    while (*current != NULL) {
        const OperatorInfo *opInfo = getOperatorInfo((*current)->type);
        if (opInfo == NULL || opInfo->precedence < minPrec) break;

        int nextMinPrec = opInfo->isRightAssociative ? opInfo->precedence : opInfo->precedence + 1;

        Token opToken = *current;
        *current = (*current)->next;
        ASTNode right = parseExpression(current, fatherType, nextMinPrec);
        if (right == NULL) return NULL;

        ASTNode opNode = createNode(opToken, opInfo->nodeType);
        opNode->children = left;
        left->brothers = right;
        left = opNode;
    }
    return left;
}

ASTNode ExpParser(Token *crrnt, NodeTypes fatherType) {
    return parseExpression(crrnt, fatherType, PREC_NONE);
}

ASTNode parseBlock(Token *current) {
    if (current == NULL || *current == NULL || (*current)->type != TokenLeftBrace) return NULL;
    *current = (*current)->next;
    ASTNode block = createNode(NULL, BLOCK_STATEMENT);
    ASTNode lastChild = NULL;
    while (*current != NULL && (*current)->type != TokenRightBrace) {
        ASTNode statement = parseStatement(current);
        if (statement != NULL) {
            if (block->children == NULL) block->children = statement;
            else lastChild->brothers = statement;
            lastChild = statement;
        }
    }
    if (*current == NULL || (*current)->type != TokenRightBrace) {
         repError(ERROR_INVALID_EXPRESSION, "Unmatched '{'");
         freeAST(block);
         return NULL;
    }
    *current = (*current)->next;
    return block;
}

ASTNode parseStatement(Token *current) {
    if (current == NULL || *current == NULL) return NULL;

    if ((*current)->type == TokenLeftBrace) {
        return parseBlock(current);
    }

    NodeTypes decType = getDecType((*current)->type);
    if (decType != null_NODE) {
        *current = (*current)->next;
        if (*current == NULL || !isValidVariable((*current)->value)) {
            repError(ERROR_INVALID_EXPRESSION, "Expected identifier after type");
            return NULL;
        }
        ASTNode decNode = createNode(*current, decType);
        *current = (*current)->next;
        if (*current != NULL && (*current)->type == TokenAssignement) {
            *current = (*current)->next;
            decNode->children = ExpParser(current, decType);
            if(decNode->children == NULL) {
                freeAST(decNode);
                return NULL;
            }
        }
        if (*current != NULL && (*current)->type == TokenPunctuation) {
            *current = (*current)->next;
        } else {
            // FIXED: Don't pass the error message as the invalid expression
            repError(ERROR_INVALID_EXPRESSION, (*current != NULL) ? (*current)->value : "EOF");
            freeAST(decNode);
            return NULL;
        }
        return decNode;
    }

    // Check if we're at a semicolon without any expression before it
    if ((*current)->type == TokenPunctuation) {
        // This is an empty statement (just a semicolon), skip it
        *current = (*current)->next;
        return NULL; // Return NULL to indicate no statement was parsed
    }

    ASTNode expressionNode = ExpParser(current, null_NODE);
    if(expressionNode) {
        if (*current != NULL && (*current)->type == TokenPunctuation) {
            *current = (*current)->next;
        } else {
            // FIXED: Don't pass the error message as the invalid expression
            repError(ERROR_INVALID_EXPRESSION, (*current != NULL) ? (*current)->value : "EOF");
            freeAST(expressionNode);
            return NULL;
        }
        return expressionNode;
    }
    
    // If we reach here, we couldn't parse anything valid
    if (*current != NULL) {
        // Skip the problematic token to avoid infinite loops
        *current = (*current)->next;
    }
    return NULL;
}

ASTNode ASTGenerator(Token token) {
    if (token == NULL) return NULL;
    ASTNode programNode = createNode(NULL, PROGRAM); 
    ASTNode lastStatement = NULL;
    Token current = token->next;
    while (current != NULL) {
        ASTNode currentStatement = parseStatement(&current);
        if (currentStatement != NULL) { // Only add non-NULL statements
            if (programNode->children == NULL) {
                programNode->children = currentStatement;
            } else {
                lastStatement->brothers = currentStatement;
            }
            
            ASTNode tail = currentStatement;
            while (tail != NULL && tail->brothers != NULL) tail = tail->brothers;
            lastStatement = tail;
        }
        // If currentStatement is NULL, just continue to the next iteration
        // This handles empty statements (standalone semicolons) gracefully
    }
    return programNode;
}

// --- UTILITY FUNCTIONS ---
void printASTTree(ASTNode node, char *prefix, int isLast) {
    if (node == NULL) return;
    char *nodeTypeStr;
    switch (node->NodeType) {
        case PROGRAM: nodeTypeStr = "PROGRAM"; break;
        case STRING_VARIABLE_DEFINITION: nodeTypeStr = "STRING_VAR_DEF"; break;
        case INT_VARIABLE_DEFINITION: nodeTypeStr = "INT_VAR_DEF"; break;
        case FLOAT_VARIABLE_DEFINITION: nodeTypeStr = "FLOAT_VAR_DEF"; break;
        case BOOL_VARIABLE_DEFINITION: nodeTypeStr = "BOOL_VAR_DEF"; break;
        case STRING_LIT: nodeTypeStr = "STRING_LIT"; break;
        case INT_LIT: nodeTypeStr = "INT_LIT"; break;
        case ADD_OP: nodeTypeStr = "ADD_OP"; break;
        case SUB_OP: nodeTypeStr = "SUB_OP"; break;
        case MUL_OP: nodeTypeStr = "MUL_OP"; break;
        case DIV_OP: nodeTypeStr = "DIV_OP"; break;
        case MOD_OP: nodeTypeStr = "MOD_OP"; break;
        case VARIABLE: nodeTypeStr = "VARIABLE"; break;
        case FLOAT_LIT: nodeTypeStr = "FLOAT_LIT"; break;
        case BOOL_LIT: nodeTypeStr = "BOOL_LIT"; break;
        case ASSIGNMENT: nodeTypeStr = "ASSIGNMENT"; break;
        case PRE_INCREMENT: nodeTypeStr = "PRE_INCREMENT"; break;
        case PRE_DECREMENT: nodeTypeStr = "PRE_DECREMENT"; break;
        case POST_INCREMENT: nodeTypeStr = "POST_INCREMENT"; break;
        case POST_DECREMENT: nodeTypeStr = "POST_DECREMENT"; break;
        case COMPOUND_ADD_ASSIGN: nodeTypeStr = "COMPOUND_ADD_ASSIGN"; break;
        case COMPOUND_SUB_ASSIGN: nodeTypeStr = "COMPOUND_SUB_ASSIGN"; break;
        case COMPOUND_MUL_ASSIGN: nodeTypeStr = "COMPOUND_MULT_ASSIGN"; break;
        case COMPOUND_DIV_ASSIGN: nodeTypeStr = "COMPOUND_DIV_ASSIGN"; break;
        case LOGIC_AND: nodeTypeStr = "LOGIC_AND"; break;
        case LOGIC_OR: nodeTypeStr = "LOGIC_OR"; break;
        case LOGIC_NOT: nodeTypeStr = "LOGIC_NOT"; break;
        case EQUAL_OP: nodeTypeStr = "EQUAL_OP"; break;
        case NOT_EQUAL_OP: nodeTypeStr = "NOT_EQUAL_OP"; break;
        case LESS_THAN_OP: nodeTypeStr = "LESS_THAN_OP"; break;
        case GREATER_THAN_OP: nodeTypeStr = "GREATER_THAN_OP"; break;
        case LESS_EQUAL_OP: nodeTypeStr = "LESS_EQUAL_OP"; break;
        case GREATER_EQUAL_OP: nodeTypeStr = "GREATER_EQUAL_OP"; break;
        case UNARY_MINUS_OP: nodeTypeStr = "UNARY_MINUS_OP"; break;
        case BLOCK_STATEMENT: nodeTypeStr = "BLOCK_STATEMENT"; break;
        case null_NODE: nodeTypeStr = "PROGRAM_ROOT"; break;
        default: nodeTypeStr = "UNKNOWN"; break;
    }

    printf("%s%s%s", prefix, isLast ? "└── " : "├── ", nodeTypeStr);
    if (node->value) {
        printf(": %s", node->value);
    }
    printf("\n");

    char newPrefix[256];
    sprintf(newPrefix, "%s%s", prefix, isLast ? "    " : "│   ");

    ASTNode child = node->children;
    while (child != NULL) {
        printASTTree(child, newPrefix, child->brothers == NULL);
        child = child->brothers;
    }
}

void printAST(ASTNode node, int depth) {
    (void)depth; // depth is unused
    if (node == NULL || (node->NodeType != PROGRAM && node->NodeType != null_NODE)) {
        printf("Empty or invalid AST.\n");
        return;
    }
    printf("AST:\n");
    ASTNode child = node->children;
    while (child != NULL) {
        printASTTree(child, "", child->brothers == NULL);
        child = child->brothers;
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
