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

#include <string.h>

ASTNode parseStatement(Token *current); // Forward declaration
ASTNode parseConditional(Token *current, ASTNode condition);

const StatementHandler statementHandlers[] = {
	{TK_FN, parseFunction},
	{TK_RETURN, parseReturnStatement},
	{TK_WHILE, parseLoop},
	{TK_LBRACE, parseBlock},
	{TK_NULL, NULL}  // Sentinel
};

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
 * @return AST node for the primary expression or NULL on error
 *
 * @note Advances the token pointer past the consumed token
 */
ASTNode parsePrimaryExp(TokenList * list, size_t *pos) {
	if (*pos >= list->count) return NULL;
	Token *token = &list->tokens[*pos];

	if (detectLitType(tokenToString(token)) == VARIABLE &&
		(*pos + 1 < list->count) && list->tokens[*pos + 1].type == TK_LPAREN) {
		char *functionName = tokenToString(token);
		ADVANCE_TOKEN(list, pos);
		ASTNode funcCall = parseFunctionCall(list, pos, functionName);
		free(functionName);
		return funcCall;
		}

	ASTNode node = createValNode(token);
	if (node) ADVANCE_TOKEN(list, pos);
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
ASTNode parseUnary(Token *current) {
	if (!current || !*current) return NULL;

	// Handle prefix operators
	if ((*current)->type == TK_MINUS || (*current)->type == TK_NOT ||
		(*current)->type == TK_INCR || (*current)->type == TK_DECR ||
		(*current)->type == TK_PLUS) {
		Token opToken = *current;
		ADVANCE_TOKEN(current);

		ASTNode operand, opNode;
		PARSE_OR_CLEANUP(operand, parseUnary(current));

		NodeTypes opType = getUnaryOpType(opToken->type);
		if (opType == null_NODE) return NULL;

		CREATE_NODE_OR_FAIL(opNode, opToken, opType);
		opNode->children = operand;
		return opNode;
		}

	// Parse primary expression
	ASTNode node = parsePrimaryExp(current);
	if (!node) return NULL;  // Return early if parsing failed

	// Check for postfix operators
	if (*current && ((*current)->type == TK_INCR ||
					 (*current)->type == TK_DECR)) {
		Token opToken = *current;
		ADVANCE_TOKEN(current);

		ASTNode opNode;
		NodeTypes opType = (opToken->type == TK_INCR) ? POST_INCREMENT : POST_DECREMENT;
		CREATE_NODE_OR_FAIL(opNode, opToken, opType);
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
 * @param minPrec Minimum precedence level required for operators
 * @return AST node representing the parsed expression
 *
 * @note Handles both left-associative (a+b+c = (a+b)+c) and
 *       right-associative (a=b=c = a=(b=c)) operators correctly
 */
ASTNode parseExpression(Token *current, Precedence minPrec) {
    if (current == NULL || *current == NULL) return NULL;
    ASTNode left = parseUnary(current);
    if (left == NULL) return NULL;
    while (*current != NULL) {
        if ((*current)->type == TK_QUESTION && PREC_TERNARY >= minPrec) {
            left = parseConditional(current, left);
            if (left == NULL) return NULL;
            continue;
        }
        const OperatorInfo *opInfo = getOperatorInfo((*current)->type);
        if (opInfo == NULL || opInfo->precedence < minPrec) break;

        Precedence nextMinPrec = opInfo->isRightAssociative ? opInfo->precedence : opInfo->precedence + 1;

        Token opToken = *current;
        ADVANCE_TOKEN(current);
        ASTNode right = parseExpression(current, nextMinPrec);
        if (right == NULL) return NULL;

        ASTNode opNode = createNode(opToken, opInfo->nodeType);
        opNode->children = left;
        left->brothers = right;
        left = opNode;
    }
    return left;
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
	EXPECT_TOKEN(current, TK_LBRACE, "Expected '{'");
    ADVANCE_TOKEN(current);
	ASTNode block;
	CREATE_NODE_OR_FAIL(block, NULL, BLOCK_STATEMENT);

	ASTNode lastChild = NULL;
	while (*current && (*current)->type != TK_RBRACE) {
		ASTNode statement = parseStatement(current);
		if (statement) {
			if (!block->children) block->children = statement;
			else if (lastChild) lastChild->brothers = statement;
			lastChild = statement;
		}
	}

	EXPECT_AND_ADVANCE(current, TK_RBRACE, "Missing closing brace '}'");
	return block;
}

/**
 * @brief Converts existing BLOCK_STATEMENT to BLOCK_EXPRESSION for ternary use.
 *
 * Reuses existing parseBlock() function and converts the node type
 * to BLOCK_EXPRESSION for use in ternary operators.
 *
 * @param current Pointer to current token (advanced during parsing)
 * @return BLOCK_EXPRESSION AST node or NULL on error
 */
ASTNode parseBlockExpression(Token *current) {
	ASTNode block = parseBlock(current);
	if (block) block->nodeType = BLOCK_EXPRESSION;
	return block;
}

/**
* @brief Parses conditional expressions with optional else clause (? syntax).
*
* Handles if-else constructs using ? syntax:
* - condition ? action (if-only)
* - condition ? action1 : action2 (if-else)
* - condition ? { block } : { block } (with blocks)
*/
ASTNode parseConditional(Token *current, ASTNode condition) {
	EXPECT_TOKEN(current, TK_QUESTION, "Expected '?'");
	Token questionToken = *current;
	ADVANCE_TOKEN(current);

	ASTNode trueBranch = (*current && (*current)->type == TK_LBRACE)
		? parseBlockExpression(current)
		: parseExpression(current, PREC_NONE);

	if (!trueBranch) {
		repError(ERROR_TERNARY_INVALID_CONDITION, NULL);
		freeAST(condition);
		return NULL;
	}

	ASTNode falseBranch = NULL;
	if (*current && (*current)->type == TK_COLON) {
		ADVANCE_TOKEN(current);
		PARSE_OR_CLEANUP(falseBranch,
			(*current && (*current)->type == TK_LBRACE)
				? parseBlockExpression(current)
				: parseExpression(current, PREC_NONE),
			condition, trueBranch);
	}

	ASTNode conditionalNode, trueBranchWrap;
	CREATE_NODE_OR_FAIL(conditionalNode, questionToken, IF_CONDITIONAL);
	CREATE_NODE_OR_FAIL(trueBranchWrap, NULL, IF_TRUE_BRANCH);

	conditionalNode->children = condition;
	trueBranchWrap->children = trueBranch;
	condition->brothers = trueBranchWrap;

	if (falseBranch) {
		ASTNode falseBranchWrap;
		CREATE_NODE_OR_FAIL(falseBranchWrap, NULL, ELSE_BRANCH);
		falseBranchWrap->children = falseBranch;
		trueBranchWrap->brothers = falseBranchWrap;
	}

	return conditionalNode;
}

/**
* @brief Parses while loop statements using @ syntax.
*
* Handles while loop constructs:
* - @ condition { statements } (basic while)
* - @ complex_expr { body; } (any expression condition)
*
* Can be combined with declarations for for-like patterns:
* int i = 0; @ i < 10; { i++; process(i); }
*
* @param current Pointer to current token (advanced during parsing)
* @return LOOP_STATEMENT AST node or NULL on error
*
* @note Condition can be any expression. Body must be block statement.
*/
ASTNode parseLoop(Token* current) {
    if (!current || !*current) return NULL;
    Token loopToken = *current;
    ADVANCE_TOKEN(current);

	ASTNode condition, loopBody, loopNode;
	PARSE_OR_CLEANUP(condition, parseExpression(current, PREC_NONE));
	EXPECT_TOKEN(current, TK_LBRACE, "Expected '{' after loop condition");
	PARSE_OR_CLEANUP(loopBody, parseBlock(current));
	CREATE_NODE_OR_FAIL(loopNode, loopToken, LOOP_STATEMENT);

    loopNode->children = condition;
    condition->brothers = loopBody;
    return loopNode;
}

ASTNode parseParameter(Token * current) {
    if (!current || !*current || detectLitType((*current)->value) != VARIABLE) {
        repError(ERROR_INVALID_EXPRESSION, "Expected parameter name");
        return NULL;
    }
    ASTNode paramNode, typeNode;
	CREATE_NODE_OR_FAIL(paramNode, *current, PARAMETER);
    ADVANCE_TOKEN(current);

    EXPECT_AND_ADVANCE(current, TK_COLON, "Expected ':' after parameter name");
	if (!*current || !isTypeToken((*current)->type)) {
		repError(ERROR_INVALID_EXPRESSION, "Expected type after ':'");
		freeAST(paramNode);
		return NULL;
	}
	Token typeToken = *current;
	ADVANCE_TOKEN(current);
	CREATE_NODE_OR_FAIL(typeNode, typeToken, getDecType(typeToken->type));
	paramNode->children = typeNode;
	return paramNode;
}

// wrapper so parseCommaSeparatedLists() general function works both types
ASTNode parseArg(Token * current) {
	return parseExpression(current, PREC_NONE);
}

ASTNode parseCommaSeparatedLists(Token *current, NodeTypes listType,
								 ASTNode (*parseElement)(Token*)) {
	EXPECT_AND_ADVANCE(current, TK_LPAREN, "Expected '('");

	ASTNode listNode;
	CREATE_NODE_OR_FAIL(listNode, NULL, listType);

	ASTNode last = NULL;
	while (*current && (*current)->type != TK_RPAREN) {
		ASTNode elem;
		PARSE_OR_CLEANUP(elem, parseElement(current), listNode);

		if (!listNode->children) listNode->children = elem;
		else last->brothers = elem;
		last = elem;

		if (*current && (*current)->type == TK_COMMA) {
			ADVANCE_TOKEN(current);
		} else if ((*current)->type != TK_RPAREN) {
			repError(ERROR_INVALID_EXPRESSION, "Expected ',' or ')'");
			freeAST(listNode);
			return NULL;
		}
	}

	EXPECT_AND_ADVANCE(current, TK_RPAREN, "Expected ')'");
	return listNode;
}

ASTNode parseReturnType(Token *current) {
	EXPECT_AND_ADVANCE(current, TK_ARROW, "Expected '->'");

	if (!*current || !isTypeToken((*current)->type)) {
		repError(ERROR_INVALID_EXPRESSION, "Expected type after '->'");
		return NULL;
	}

	Token typeToken = *current;
	NodeTypes returnType = getReturnTypeFromToken(typeToken->type);
	ADVANCE_TOKEN(current);

	ASTNode returnTypeNode, typeNode;
	CREATE_NODE_OR_FAIL(returnTypeNode, typeToken, RETURN_TYPE);

	if (returnType != null_NODE) {
		CREATE_NODE_OR_FAIL(typeNode, typeToken, returnType);
		returnTypeNode->children = typeNode;
	}

	return returnTypeNode;
}

ASTNode parseFunctionCall(TokenList * list, size_t * pos, char *functionName) {
	EXPECT_TOKEN(list, pos, TK_LPAREN, "Expected '(' for function call");

	ASTNode callNode, argList;
	CREATE_NODE_OR_FAIL(callNode, NULL, FUNCTION_CALL);
	callNode->value = strdup(functionName);

	PARSE_OR_CLEANUP(argList, parseCommaSeparatedLists(current, ARGUMENT_LIST, parseArg),
					 callNode);

	callNode->children = argList;
	return callNode;
}

ASTNode parseReturnStatement(Token *current) {
	EXPECT_TOKEN(current, TK_RETURN, "Expected 'return' keyword");
	Token returnToken = *current;
	ADVANCE_TOKEN(current);

	ASTNode returnNode;
	CREATE_NODE_OR_FAIL(returnNode, returnToken, RETURN_STATEMENT);

	if (*current && (*current)->type != TK_SEMI) {
		returnNode->children = parseExpression(current, PREC_NONE);
	}

	EXPECT_AND_ADVANCE(current, TK_SEMI, "Expected ';' after return statement");
	return returnNode;
}

ASTNode parseFunction(Token *current) {
	EXPECT_TOKEN(current, TK_FN, "Expected 'fn'");
	Token fnToken = *current;
	ADVANCE_TOKEN(current);

	if (!*current || detectLitType((*current)->value) != VARIABLE) {
		repError(ERROR_INVALID_EXPRESSION, "Expected function name after 'fn'");
		return NULL;
	}

	ASTNode functionNode;
	CREATE_NODE_OR_FAIL(functionNode, fnToken, FUNCTION_DEFINITION);
	functionNode->value = strdup((*current)->value);
	ADVANCE_TOKEN(current);

	ASTNode paramList, returnType, body;
	PARSE_OR_CLEANUP(paramList, parseCommaSeparatedLists(current, PARAMETER_LIST, parseParameter),
					 functionNode);
	PARSE_OR_CLEANUP(returnType, parseReturnType(current), functionNode, paramList);
	EXPECT_TOKEN(current, TK_LBRACE, "Expected '{' for function body");
	PARSE_OR_CLEANUP(body, parseBlock(current), functionNode, paramList, returnType);

	functionNode->children = paramList;
	paramList->brothers = returnType;
	returnType->brothers = body;

	return functionNode;
}

static ASTNode parseDeclaration(Token *current, NodeTypes decType) {
	if (!*current || detectLitType((*current)->value) != VARIABLE) {
		repError(ERROR_INVALID_EXPRESSION, "Expected identifier after type");
		return NULL;
	}

	ASTNode decNode;
	CREATE_NODE_OR_FAIL(decNode, *current, decType);
	ADVANCE_TOKEN(current);

	if (*current && (*current)->type == TK_ASSIGN) {
		ADVANCE_TOKEN(current);
		PARSE_OR_CLEANUP(decNode->children, parseExpression(current, PREC_NONE), decNode);
	}

	EXPECT_AND_ADVANCE(current, TK_SEMI, "Expected ';'");
	return decNode;
}

static ASTNode parseExpressionStatement(Token *current) {
	ASTNode expressionNode;
	PARSE_OR_CLEANUP(expressionNode, parseExpression(current, PREC_NONE));
	EXPECT_AND_ADVANCE(current, TK_SEMI, "Expected ';'");
	return expressionNode;
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
 * Statement := VarDecl | Block | ExprStatement | EmptyStatement | loop
 * VarDecl   := Type IDENTIFIER ('=' Expression)? ';'
 * Block     := '{' Statement* '}'
 * ExprStatement := Expression ';'
 * EmptyStatement := ';'
 * Loop = @ condition { code } ';'
 * ```
 *
 * @note Returns NULL for empty statements (standalone semicolons) which
 *       are silently skipped by the caller
 */
ASTNode parseStatement(Token *current) {
	if (!current || !*current) return NULL;

	if ((*current)->type == TK_SEMI) {
		ADVANCE_TOKEN(current);
		return NULL;
	}

	for (int i = 0; statementHandlers[i].token != TK_NULL; i++) {
		if ((*current)->type == statementHandlers[i].token) {
			return statementHandlers[i].handler(current);
		}
	}

	NodeTypes decType = getDecType((*current)->type);
	if (decType != null_NODE) {
		ADVANCE_TOKEN(current);
		return parseDeclaration(current, decType);
	}

	return parseExpressionStatement(current);
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
ASTNode ASTGenerator(TokenType * tokenList) {
	if (!tokenList) return NULL;

	ASTNode programNode;
	CREATE_NODE_OR_FAIL(programNode, NULL, PROGRAM);

	Token current = token->next;  // Skip dummy head
	ASTNode lastStatement = NULL;

	while (current) {
		ASTNode currentStatement = parseStatement(&current);
		if (currentStatement) {
			if (!programNode->children) {
				programNode->children = currentStatement;
			} else if (lastStatement) {
				lastStatement->brothers = currentStatement;
			}

			// Find the tail of the statement chain
			ASTNode tail = currentStatement;
			while (tail && tail->brothers) tail = tail->brothers;
			lastStatement = tail;
		}
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

    const char *nodeTypeStr = getNodeTypeName(node->nodeType);

    printf("%s%s%s", prefix, isLast ? "|___ " : "|-- ", nodeTypeStr);
    if (node->value) {
        printf(": %s", node->value);
    }
    printf("\n");

    char newPrefix[256];
    sprintf(newPrefix, "%s%s", prefix, isLast ? "    " : "|   ");

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
    if (node == NULL || (node->nodeType != PROGRAM && node->nodeType != null_NODE)) {
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
