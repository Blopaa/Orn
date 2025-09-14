/**
* @file parserHelpers.c
 * @brief Helper functions and utilities for the parser module.
 *
 * Contains validation functions, AST node creation utilities, and lookup
 * functions that support the main parser implementation. These functions
 * provide:
 * - Token type to AST node type conversion
 * - Literal value validation and classification
 * - AST node construction and initialization
 * - Operator precedence lookup for Pratt parser
 * - Node type name mapping for debugging output
 *
 * This module isolates commonly used utility functions to improve
 * code organization and maintainability.
 */

#include "parser.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "../errorHandling/errorHandling.h"
#include "../lexer/lexer.h"
// --- HELPER FUNCTIONS ---
/**
 * @brief Converts a token type to its corresponding AST declaration node type.
 *
 * Looks up the token type in the TypeDefs mapping table to find the
 * corresponding AST node type for variable declarations.
 *
 * @param type Token type from lexer (e.g., TokenIntDefinition)
 * @return Corresponding AST node type (e.g., INT_VARIABLE_DEFINITION) or null_NODE
 *
 * @note Used primarily for converting type keywords (int, string, etc.)
 *       to their declaration node types during parsing.
 */
NodeTypes getDecType(TokenType type) {
    for (int i = 0; TypeDefs[i].TkType != TokenNULL; i++) {
        if (TypeDefs[i].TkType == type) return TypeDefs[i].type;
    }
    return null_NODE;
}

/**
 * @brief Unified literal type detection with optimized checks.
 * Single function replaces all validation functions.
 */
NodeTypes detectLitType(const char *val) {
	if (!val) return null_NODE;

	size_t len = strlen(val);
	if (len >= 2 && val[0] == '"' && val[len-1] == '"')
		return STRING_LIT;

	if (!strcmp(val, "true") || !strcmp(val, "false"))
		return BOOL_LIT;

	int start = (val[0] == '-') ? 1 : 0;
	if (!val[start]) goto check_variable;

	int has_dot = 0, all_digits = 1;
	for (int i = start; val[i]; i++) {
		if (val[i] == '.') {
			if (has_dot) { all_digits = 0; break; }
			has_dot = 1;
		} else if (!isdigit(val[i])) {
			all_digits = 0;
			break;
		}
	}

	if (all_digits)
		return has_dot ? FLOAT_LIT : INT_LIT;

	check_variable:
		if (isalpha(val[0]) || val[0] == '_') {
			for (int i = 1; val[i]; i++) {
				if (!isalnum(val[i]) && val[i] != '_') {
					repError(ERROR_INVALID_EXPRESSION, val);
					return null_NODE;
				}
			}
			return VARIABLE;
		}

	repError(ERROR_INVALID_EXPRESSION, val);
	return null_NODE;
}

/**
 * @brief Converts AST node type to human-readable string for debugging.
 *
 * Looks up the node type in the nodeTypeMapping table to find the
 * corresponding display name. Used primarily for AST visualization
 * and debugging output to provide meaningful node type names.
 *
 * @param nodeType AST node type to convert
 * @return Human-readable string representation of the node type
 *
 * @note Returns "UNKNOWN" for unrecognized node types. The nodeTypeMapping
 *       table should be kept in sync with the NodeTypes enumeration.
 */
const char *getNodeTypeName(NodeTypes nodeType) {
    for (int i = 0; nodeTypeMapping[i].displayName != NULL; i++) {
        if (nodeTypeMapping[i].nodeType == nodeType) {
            return nodeTypeMapping[i].displayName;
        }
    }
    return "UNKNOWN";
}

/**
 * @brief Creates a new AST node with given token and type information.
 *
 * Allocates and initializes a new AST node with:
 * - Copied value string from token (if present)
 * - Specified node type
 * - Position information for error reporting
 * - Initialized child and sibling pointers
 *
 * @param token Source token (can be NULL for synthetic nodes)
 * @param type AST node type to assign
 * @return Newly allocated AST node or NULL on allocation failure
 *
 * @note The value string is duplicated, so the original token can be freed.
 *       The caller is responsible for freeing the returned node.
 */
ASTNode createNode(Token token, NodeTypes type) {
    ASTNode node = malloc(sizeof(struct ASTNode));
	if (!node) {
		repError(ERROR_MEMORY_ALLOCATION_FAILED, token && token->value ? token->value : "");
		return NULL;
	}

    node->value = (token && token->value) ? strdup(token->value) : NULL;
    node->nodeType = type;

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

/**
 * @brief Creates appropriate AST node based on token value and context.
 *
 * Analyzes the token value to determine the correct literal type and
 * creates the corresponding AST node. Handles type inference for:
 * - String literals (quoted strings)
 * - Float literals (contains decimal point)
 * - Integer literals (all digits)
 * - Boolean literals (true/false)
 * - Variable references (valid identifiers)
 *
 * @param current_token Token to convert to AST node
 * @return Newly created AST node or NULL on error
 *
 * @note Reports ERROR_INVALID_EXPRESSION for unrecognized token formats
 */
ASTNode createValNode(Token current_token) {
    if (current_token == NULL) return NULL;
    NodeTypes type = detectLitType(current_token->value);
    if (type == null_NODE) return NULL;
	return createNode(current_token, type);
}

/**
 * @brief Looks up operator information for precedence parsing.
 *
 * Searches the static operators array to find the OperatorInfo
 * structure corresponding to the given token type.
 *
 * @param type Token type to look up
 * @return Pointer to OperatorInfo structure or NULL if not found
 *
 * @note Used by the Pratt parser to determine operator precedence
 *       and associativity during expression parsing.
 */
const OperatorInfo *getOperatorInfo(TokenType type) {
    for (int i = 0; operators[i].token != TokenNULL; i++) {
        if (operators[i].token == type) return &operators[i];
    }
    return NULL;
}

/**
 * @brief Checks if a token represents a valid type.
 *
 * @param type Token type to check
 * @return 1 if it's a type token, 0 otherwise
 */
int isTypeToken(TokenType type) {
    return (type == TokenIntDefinition ||
            type == TokenStringDefinition ||
            type == TokenFloatDefinition ||
            type == TokenBoolDefinition ||
            type == TokenVoidDefinition);
}

/**
 * @brief Converts a token type to corresponding AST return type node.
 *
 * @param type Token type (e.g., TokenIntDefinition)
 * @return Corresponding AST node type (e.g., INT_VARIABLE_DEFINITION)
 */
NodeTypes getReturnTypeFromToken(TokenType type) {
    switch (type) {
    case TokenIntDefinition: return INT_VARIABLE_DEFINITION;
    case TokenStringDefinition: return STRING_VARIABLE_DEFINITION;
    case TokenFloatDefinition: return FLOAT_VARIABLE_DEFINITION;
    case TokenBoolDefinition: return BOOL_VARIABLE_DEFINITION;
    default: return null_NODE;
    }
}

NodeTypes getUnaryOpType(TokenType t) {
	switch (t) {
		case TokenSub: return UNARY_MINUS_OP;
		case TokenSum: return UNARY_PLUS_OP;
		case TokenNot: return LOGIC_NOT;
		case TokenIncrement: return PRE_INCREMENT;
		case TokenDecrement: return PRE_DECREMENT;
		default: return null_NODE;
	}
}