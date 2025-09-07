/**
* @file parser.c
 * @brief Implementation of recursive descent parser with operator precedence.
 *
 * Implements a hybrid parsing approach combining:
 * - Recursive descent parsing for statements and declarations
 * - Pratt parser (operator precedence parser) for expressions
 *
 * Key features:
 * - Proper operator precedence and associativity handling
 * - Block statement parsing with nested scopes
 * - Error recovery and detailed error reporting
 * - Memory-safe AST construction and cleanup
 * - Support for complex expressions with mixed operators
 *
 * The parser transforms a stream of tokens from the lexer into a complete
 * Abstract Syntax Tree (AST) ready for semantic analysis or code generation.
 */

#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

#include "../errorHandling/errorHandling.h"
#include "../lexer/lexer.h"

ASTNode parseStatement(Token *current); // Forward declaration


// --- PARSER IMPLEMENTATION ---
/**
 * @brief Parses primary expressions (literals, identifiers, parentheses).
 *
 * Handles the lowest level of expression parsing for atomic values:
 * - Integer and float literals
 * - String literals
 * - Boolean literals
 * - Variable identifiers
 * - Parenthesized expressions (future enhancement)
 *
 * This is the base case for the recursive expression parser, handling
 * the fundamental building blocks of expressions.
 *
 * @param current Pointer to current token (updated after consumption)
 * @param fatherType Parent node type for context
 * @return AST node for the primary expression or NULL on error
 *
 * @note Advances the token pointer past the consumed token
 */
ASTNode parsePrimaryExp(Token *current, NodeTypes fatherType) {
    if (current == NULL || *current == NULL) return NULL;
    ASTNode node = createValNode(*current, fatherType);
    if (node != NULL) {
        *current = (*current)->next;
    }
    return node;
}

/**
 * @brief Parses unary expressions and postfix operators.
 *
 * Handles prefix unary operators and postfix increment/decrement:
 * - Prefix: !, ++, --, unary -
 * - Postfix: ++, --
 *
 * Uses recursive calls for prefix operators to handle nested unary expressions
 * (e.g., !!x, ++--y) and iterative approach for postfix operators to maintain
 * correct precedence and associativity.
 *
 * @param current Pointer to current token (updated during parsing)
 * @param fatherType Parent node type for context
 * @return AST node representing the unary expression
 *
 * Grammar supported:
 * ```
 * UnaryExpr := ('!' | '++' | '--' | '-' | '+') UnaryExpr
 *            | PrimaryExpr ('++' | '--')?
 * ```
 *
 * @note Prefix operators have higher precedence than postfix operators
 */
ASTNode parseUnary(Token *current, NodeTypes fatherType) {
    if (current == NULL || *current == NULL) return NULL;
    if ((*current)->type == TokenSub || (*current)->type == TokenNot ||
        (*current)->type == TokenIncrement || (*current)->type == TokenDecrement || (*current)->type == TokenSum) {
        Token opToken = *current;
        *current = (*current)->next;
        ASTNode operand = parseUnary(current, fatherType);
        if (operand == NULL) return NULL;
        NodeTypes opType;
        if (opToken->type == TokenSub) opType = UNARY_MINUS_OP;
        else if (opToken->type == TokenSum) opType = UNARY_PLUS_OP;
        else if (opToken->type == TokenNot) opType = LOGIC_NOT;
        else if (opToken->type == TokenIncrement) opType = PRE_INCREMENT;
        else opType = PRE_DECREMENT;
        ASTNode opNode = createNode(opToken, opType);
        opNode->children = operand;
        return opNode;
    }
    ASTNode node = parsePrimaryExp(current, fatherType);
    if (node != NULL && *current != NULL && ((*current)->type == TokenIncrement || (*current)->type ==
                                             TokenDecrement)) {
        Token opToken = *current;
        *current = (*current)->next;
        NodeTypes opType = (opToken->type == TokenIncrement) ? POST_INCREMENT : POST_DECREMENT;
        ASTNode opNode = createNode(opToken, opType);
        opNode->children = node;
        return opNode;
    }
    return node;
}

/**
 * @brief Parses expressions using operator precedence climbing algorithm.
 *
 * Implements the Pratt parser algorithm for handling operator precedence
 * and associativity correctly. This is the core of expression parsing,
 * supporting complex nested expressions with proper precedence rules.
 *
 * Algorithm:
 * 1. Parse left operand (unary expression)
 * 2. While current operator has sufficient precedence:
 *    a. Determine minimum precedence for right operand
 *    b. Parse right operand recursively
 *    c. Create binary operator node
 *    d. Continue with the result as new left operand
 *
 * @param current Pointer to current token (advanced during parsing)
 * @param fatherType Parent node type for context
 * @param minPrec Minimum precedence level required for operators
 * @return AST node representing the parsed expression
 *
 * @note Handles both left-associative (a+b+c = (a+b)+c) and
 *       right-associative (a=b=c = a=(b=c)) operators correctly
 */
ASTNode parseExpression(Token *current, NodeTypes fatherType, Precedence minPrec) {
    if (current == NULL || *current == NULL) return NULL;
    ASTNode left = parseUnary(current, fatherType);
    if (left == NULL) return NULL;
    while (*current != NULL) {
        const OperatorInfo *opInfo = getOperatorInfo((*current)->type);
        if (opInfo == NULL || opInfo->precedence < minPrec) break;

        Precedence nextMinPrec = opInfo->isRightAssociative ? opInfo->precedence : opInfo->precedence + 1;

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

/**
 * @brief Entry point for expression parsing with default precedence.
 *
 * Convenient wrapper around parseExpression() that starts with the
 * lowest precedence level (PREC_NONE), allowing all operators to be parsed.
 *
 * @param crrnt Pointer to current token (advanced during parsing)
 * @param fatherType Parent node type for context
 * @return AST node representing the complete expression
 *
 * @note This is the main interface used by statement parsers to parse
 *       expressions in assignments, declarations, and standalone expressions.
 */
ASTNode ExpParser(Token *crrnt, NodeTypes fatherType) {
    return parseExpression(crrnt, fatherType, PREC_NONE);
}

/**
 * @brief Parses block statements enclosed in curly braces.
 *
 * Handles block structure parsing for scoped statements:
 * - Creates BLOCK_STATEMENT node
 * - Parses all statements within the block
 * - Links statements as siblings under the block
 * - Validates proper brace matching
 *
 * Supports:
 * - Empty blocks: {}
 * - Single statement blocks: { x = 5; }
 * - Multi-statement blocks: { int x = 1; y = x + 2; }
 * - Nested blocks: { int x = 1; { int y = 2; } }
 *
 * @param current Pointer to current token (advanced during parsing)
 * @return BLOCK_STATEMENT AST node containing all parsed statements
 *
 * @note Reports ERROR_INVALID_EXPRESSION for missing closing brace
 *       and performs proper cleanup on error conditions
 */
ASTNode parseBlock(Token *current) {
    if (current == NULL || *current == NULL || (*current)->type != TokenLeftBrace) return NULL;
    *current = (*current)->next;
    ASTNode block = createNode(NULL, BLOCK_STATEMENT);
    ASTNode lastChild = NULL;
    while (*current != NULL && (*current)->type != TokenRightBrace) {
        ASTNode statement = parseStatement(current);
        if (statement != NULL) {
            if (block->children == NULL) block->children = statement;
            else if (lastChild != NULL) lastChild->brothers = statement;
            lastChild = statement;
        }
    }
    if (*current == NULL || (*current)->type != TokenRightBrace) {
        repError(ERROR_INVALID_EXPRESSION, "Missing closing brace '}'");
        freeAST(block);
        return NULL;
    }
    *current = (*current)->next;
    return block;
}

/**
 * @brief Parses individual statements (declarations, assignments, expressions).
 *
 * Main statement parsing function that handles:
 * - Variable declarations: int x = 5;
 * - Block statements: { ... }
 * - Expression statements: x = y + z;
 * - Assignment operations: x += 10;
 * - Empty statements: ;
 *
 * Implements error recovery by continuing parsing after invalid statements
 * and provides detailed error reporting for syntax issues.
 *
 * @param current Pointer to current token (advanced during parsing)
 * @return AST node for the parsed statement or NULL for empty/invalid statements
 *
 * Grammar supported:
 * ```
 * Statement := VarDecl | Block | ExprStatement | EmptyStatement
 * VarDecl   := Type IDENTIFIER ('=' Expression)? ';'
 * Block     := '{' Statement* '}'
 * ExprStatement := Expression ';'
 * EmptyStatement := ';'
 * ```
 *
 * @note Returns NULL for empty statements (standalone semicolons) which
 *       are silently skipped by the caller
 */
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
            if (decNode->children == NULL) {
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
    if (expressionNode) {
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

/**
 * @brief Main AST generation function - parses complete token stream.
 *
 * Creates the root PROGRAM node and parses all statements in the token stream.
 * Handles multiple statements by linking them as siblings under the program node.
 * Implements error recovery by skipping invalid statements and continuing parsing.
 *
 * Process:
 * 1. Create PROGRAM root node
 * 2. Skip dummy head token from tokenization
 * 3. Parse statements sequentially until end of tokens
 * 4. Link valid statements as siblings under program
 * 5. Handle error recovery for invalid statements
 *
 * @param token Token linked list from tokenization (with dummy head)
 * @return Root PROGRAM node containing all parsed statements
 *
 * @note Returns a PROGRAM node even if parsing fails - check hasErrors()
 *       to determine if parsing was successful. The dummy head token from
 *       tokenization() is automatically skipped.
 */
ASTNode ASTGenerator(Token token) {
    if (token == NULL) return NULL;
    ASTNode programNode = createNode(NULL, PROGRAM);
    ASTNode lastStatement = NULL;
    Token current = token->next;
    while (current != NULL) {
        ASTNode currentStatement = parseStatement(&current);
        if (currentStatement != NULL) {
            // Only add non-NULL statements
            if (programNode->children == NULL) {
                programNode->children = currentStatement;
            } else if (lastStatement != NULL) {
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

/**
 * @brief Recursively prints AST tree structure with visual formatting.
 *
 * Creates a visual tree representation using Unicode box-drawing characters
 * for clear hierarchical display. Handles both child and sibling relationships
 * to show the complete tree structure.
 *
 * @param node Current AST node to print
 * @param prefix String prefix for indentation and tree lines
 * @param isLast Flag indicating if this is the last sibling (affects formatting)
 *
 * Output format uses Unicode characters:
 * - ├── for non-terminal siblings
 * - └── for terminal siblings
 * - │   for continuation lines
 * - (space) for final indentation
 *
 * Example output:
 * ```
 * ├── INT_VAR_DEF: x
 * │   └── ADD_OP
 * │       ├── INT_LIT: 5
 * │       └── INT_LIT: 3
 * └── ASSIGNMENT: y
 *     └── VARIABLE: x
 * ```
 */
void printASTTree(ASTNode node, char *prefix, int isLast) {
    if (node == NULL) return;

    const char *nodeTypeStr = getNodeTypeName(node->NodeType);

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

/**
 * @brief Main AST printing function with validation and formatting.
 *
 * Provides the main interface for displaying AST trees with proper
 * validation and error handling. Ensures the AST is valid before
 * attempting to print and provides clear error messages for invalid trees.
 *
 * @param node Root AST node to print (typically PROGRAM node)
 * @param depth Indentation depth (maintained for compatibility, unused)
 *
 * @note The depth parameter is maintained for backward compatibility
 *       but is not used in the current tree-printing implementation.
 *       Only prints trees rooted at PROGRAM or null_NODE types.
 */
void printAST(ASTNode node, int depth) {
    (void) depth; // depth is unused
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

/**
 * @brief Recursively frees an AST and all associated memory.
 *
 * Performs deep deallocation of the entire AST structure using
 * post-order traversal to ensure all child nodes are freed before
 * their parents. Handles arbitrary tree structures with both
 * child and sibling relationships.
 *
 * Memory freed includes:
 * - All child nodes (recursive)
 * - All sibling nodes (recursive)
 * - Node value string (if present)
 * - Node structure itself
 *
 * @param node Root node to free (can be NULL)
 *
 * @note Safe to call on NULL pointer. Uses post-order traversal
 *       to ensure proper cleanup order. All strings created with
 *       strdup() in createNode() are properly freed.
 */
void freeAST(ASTNode node) {
    if (node == NULL) return;

    freeAST(node->children);
    freeAST(node->brothers);

    if (node->value) {
        free(node->value);
    }
    free(node);
}
