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

// --- VALIDATION FUNCTIONS ---

/**
 * @brief Checks if a string represents a valid floating-point literal.
 *
 * Validates float format by checking for presence of decimal point.
 * Handles negative floats by skipping the minus sign.
 *
 * @param val String to validate (can be NULL)
 * @return 1 if valid float format, 0 otherwise
 *
 * Examples:
 * - "3.14" -> 1 (valid)
 * - "-2.5" -> 1 (valid)
 * - "42" -> 0 (integer, not float)
 * - NULL -> 0 (invalid)
 */
int isFloatLit(const char *val) {
    if (val == NULL) return 0;
    int start = (val[0] == '-') ? 1 : 0;
    if (val[start] == '\0') return 0;
    for (int i = start; val[i] != '\0'; i++) {
        if (val[i] == '.') return 1;
    }
    return 0;
}

/**
 * @brief Validates identifier naming rules.
 *
 * Checks if a string follows valid identifier conventions:
 * - Must start with letter or underscore
 * - Subsequent characters can be letters, digits, or underscores
 *
 * @param val String to validate (can be NULL)
 * @return 1 if valid identifier, 0 otherwise
 *
 * Examples:
 * - "variable_name" -> 1 (valid)
 * - "_private" -> 1 (valid)
 * - "123invalid" -> 0 (starts with digit)
 * - "my-var" -> 0 (contains hyphen)
 */
int isValidVariable(const char *val) {
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

/**
 * @brief Checks if a string represents a valid integer literal.
 *
 * Validates integer format, handling negative numbers by skipping
 * the leading minus sign and checking remaining digits.
 *
 * @param val String to validate (can be NULL)
 * @return 1 if valid integer format, 0 otherwise
 *
 * Examples:
 * - "42" -> 1 (valid)
 * - "-123" -> 1 (valid)
 * - "3.14" -> 0 (contains decimal)
 * - "abc" -> 0 (non-numeric)
 */
int isIntLit(const char *val) {
    if (val == NULL) return 0;
    int start = (val[0] == '-') ? 1 : 0;
    if (val[start] == '\0') return 0;
    for (int i = start; val[i] != '\0'; i++) {
        if (!isdigit(val[i])) return 0;
    }
    return 1;
}

/**
 * @brief Validates string literal format with proper quote delimiters.
 *
 * Checks that string has opening and closing quotes and minimum length.
 *
 * @param val String to validate (can be NULL)
 * @return 1 if valid string literal format, 0 otherwise
 *
 * Examples:
 * - "\"hello\"" -> 1 (valid)
 * - "\"\"" -> 1 (valid empty string)
 * - "hello" -> 0 (missing quotes)
 * - "\"unclosed -> 0 (missing closing quote)
 */
int isValidStringLit(const char *val) {
    if (val == NULL) return 0;
    size_t len = strlen(val);
    return (len >= 2 && val[0] == '"' && val[len - 1] == '"');
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
    case TokenVoidDefinition: return null_NODE;
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

NodeTypes detectLitType(const char * val) {
	if (!val) return null_NODE;
	if (isValidStringLit(val)) return STRING_LIT;
	if (isFloatLit(val)) return FLOAT_LIT;
	if (isIntLit(val)) return INT_LIT;
	if (!strcmp(val, "true") || !strcmp(val, "false")) return BOOL_LIT;
	if (isValidVariable(val)) return VARIABLE;
	repError(ERROR_INVALID_EXPRESSION, val);
	return null_NODE;
}