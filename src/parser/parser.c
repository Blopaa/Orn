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

ASTNode parseStatement(TokenList* list, size_t* pos);

/**
 * @brief Create error context by extracting source line on-demand
 */
ErrorContext *createErrorContextFromParser(TokenList *list, size_t * pos) {
    static ErrorContext context;
    static char *lastSourceLine = NULL;  // Static to persist between calls

    if (!list || *pos >= list->count) return NULL;
	size_t tempPos = list->tokens[*pos].type != TK_SEMI ? *pos-1 : *pos;
    Token *token = &list->tokens[tempPos];

    // Free previous source line
    if (lastSourceLine) {
        free(lastSourceLine);
        lastSourceLine = NULL;
    }

    // Extract source line on-demand
    lastSourceLine = extractSourceLineForToken(list, token);

    context.file = list->filename ? list->filename : "source";
    context.line = token->line;
    context.column = token->column;
    context.source = lastSourceLine;
    context.startColumn = token->column;
    context.length = token->length;

    return &context;
}

const StatementHandler statementHandlers[] = {
	{TK_FN, parseFunction},
	{TK_RETURN, parseReturnStatement},
	{TK_WHILE, parseLoop},
	{TK_LBRACE, parseBlock},
	{TK_STRUCT, parseStruct},
	{TK_IF, parseIf},
	{TK_NULL, NULL}
};

/**
 * @brief Parses primary expressions (literals, identifiers, parentheses).
 *
 * Handles the lowest level of expression parsing for atomic values:
 * - Integer and float literals
 * - String literals
 * - Boolean literals
 * - Variable identifiers
 * - Function calls
 * - Parenthesized expressions
 *
 * @param list Token list
 * @param pos Current position in token list (updated after consumption)
 * @return AST node for the primary expression or NULL on error
 */
ASTNode parsePrimaryExp(TokenList * list, size_t *pos) {
	if (*pos >= list->count) return NULL;
	Token *token = &list->tokens[*pos];

	// Check for array literal
	if (token->type == TK_LBRACKET) {
		return parseArrLit(list, pos);
	}

	if (detectLitType(token, list, pos) == VARIABLE &&
		(*pos + 1 < list->count) && list->tokens[*pos + 1].type == TK_LPAREN) {
		Token * fnNameTok = token;
		ADVANCE_TOKEN(list, pos);
		ASTNode funcCall = parseFunctionCall(list, pos, fnNameTok);
		return funcCall;
		}

	ASTNode node = createValNode(token, list, pos);
	if (node){ ADVANCE_TOKEN(list, pos);}
	else {
		ADVANCE_TOKEN(list, pos);
		return NULL;
	}

	// Handle member access and array access
	while (*pos < list->count) {
		if (list->tokens[*pos].type == TK_DOT) {
			ADVANCE_TOKEN(list, pos); // consume '.'

			if (detectLitType(&list->tokens[*pos], list, pos) != VARIABLE) {
				reportError(ERROR_INVALID_EXPRESSION, createErrorContextFromParser(list, pos),
						   "Expected member name after '.'");
				freeAST(node);
				return NULL;
			}

			Token *memberToken = &list->tokens[*pos];
			ADVANCE_TOKEN(list, pos);

			ASTNode memberAccess, memberNode;
			CREATE_NODE_OR_FAIL(memberAccess, memberToken, MEMBER_ACCESS, list, pos);
			CREATE_NODE_OR_FAIL(memberNode, memberToken, VARIABLE, list, pos);

			memberAccess->children = node;
			node->brothers = memberNode;
			node = memberAccess;
		}
		else if (list->tokens[*pos].type == TK_LBRACKET) {
			node = parseArrayAccess(list, pos, node);
			if (!node) return NULL;
		} else {
			break;
		}
	}

	return node;
}

/**
 * @brief Parses unary expressions and postfix operators.
 *
 * Handles prefix unary operators and postfix increment/decrement.
 *
 * @param list Token list
 * @param pos Current position in token list
 * @return AST node representing the unary expression
 */
ASTNode parseUnary(TokenList * list, size_t *pos) {
	if (*pos >= list->count) return NULL;
	Token *token = &list->tokens[*pos];

	// Handle prefix operators
	if (token->type == TK_MINUS || token->type == TK_NOT ||
		token->type == TK_INCR || token->type == TK_DECR ||
		token->type == TK_PLUS) {
		Token * opToken = token;
		ADVANCE_TOKEN(list, pos);

		ASTNode operand, opNode;
		PARSE_OR_FAIL(operand,parseUnary(list, pos));

		NodeTypes opType = getUnaryOpType(opToken->type);
		if (opType == null_NODE) return NULL;

		CREATE_NODE_OR_FAIL(opNode, opToken, opType, list, pos);
		opNode->children = operand;
		return opNode;
	}

	// Parse primary expression
	ASTNode node = parsePrimaryExp(list, pos);
	if (!node) return NULL;  // Return early if parsing failed

	// Check for postfix operators
	if (*pos < list->count && (list->tokens[*pos].type == TK_INCR ||
								list->tokens[*pos].type == TK_DECR)) {
		Token * opToken = &list->tokens[*pos];
		ADVANCE_TOKEN(list, pos);

		ASTNode opNode;
		NodeTypes opType = (opToken->type == TK_INCR) ? POST_INCREMENT : POST_DECREMENT;
		CREATE_NODE_OR_FAIL(opNode, opToken, opType, list, pos);
		opNode->children = node;
		return opNode;
					 }

	return node;
}

ASTNode parseCastExpression(TokenList *list, size_t *pos, ASTNode node) {
    if (*pos >= list->count || !node) return NULL;
	Token *asTok = &list->tokens[*pos];
    ADVANCE_TOKEN(list, pos);
    Token *currentToken = &list->tokens[*pos];
    if (isTypeToken(currentToken->type)) {
        ASTNode refTypeNode, castNode;
        CREATE_NODE_OR_FAIL(refTypeNode, currentToken, getTypeNodeFromToken(currentToken->type), list, pos);
		ADVANCE_TOKEN(list, pos);
		CREATE_NODE_OR_FAIL(castNode, asTok, CAST_EXPRESSION, list, pos);
		castNode->children = node;
		node->brothers = refTypeNode;		
		return castNode;
    }
    return NULL;
}

/**
 * @brief Parses expressions using operator precedence climbing algorithm.
 *
 * Implements the Pratt parser algorithm for handling operator precedence
 * and associativity correctly.
 *
 * @param list Token list
 * @param pos Current position in token list
 * @param minPrec Minimum precedence level required for operators
 * @return AST node representing the parsed expression
 */
ASTNode parseExpression(TokenList *list,size_t * pos, Precedence minPrec) {
	if (*pos >= list->count) return NULL;

	ASTNode left = parseUnary(list, pos);
	if (left == NULL) return NULL;

	while (*pos < list->count) {
		Token* currentToken = &list->tokens[*pos];

		if (currentToken->type == TK_QUESTION && PREC_TERNARY >= minPrec) {
			ASTNode conditionalNode = parseTernary(list, pos);
			left->brothers = conditionalNode->children; // condition->brothers = trueBranchWrap, check ParseTernary comment if forgot why this
			conditionalNode->children = left;
			left = conditionalNode;
			if (left == NULL) return NULL;
			continue;
		}

		if(currentToken->type == TK_AS && PREC_CAST >= minPrec){
			left = parseCastExpression(list, pos, left);
			if(left == NULL) return NULL;
			continue;
		}

		const OperatorInfo* opInfo = getOperatorInfo(currentToken->type);
		if (opInfo == NULL || opInfo->precedence < minPrec) break;

		Precedence nextMinPrec = opInfo->isRightAssociative ?
								opInfo->precedence : opInfo->precedence + 1;

		Token* opToken = currentToken;
		ADVANCE_TOKEN(list, pos);

		ASTNode right = parseExpression(list, pos, nextMinPrec);
		if (right == NULL) return NULL;
		ASTNode opNode = createNode(opToken, opInfo->nodeType, list, pos);
		opNode->children = left;
		left->brothers = right;
		left = opNode;
	}

	return left;
}

/**
 * @brief Parses block statements enclosed in curly braces.
 *
 * @param list Token list
 * @param pos Current position in token list
 * @return BLOCK_STATEMENT AST node containing all parsed statements
 */
ASTNode parseBlock(TokenList* list, size_t* pos) {
	EXPECT_TOKEN(list, pos, TK_LBRACE, ERROR_EXPECTED_OPENING_BRACE, "Expected '{'");
	ADVANCE_TOKEN(list, pos);

	ASTNode block;
	CREATE_NODE_OR_FAIL(block, NULL, BLOCK_STATEMENT, list, pos);

	ASTNode lastChild = NULL;
	while (*pos < list->count && list->tokens[*pos].type != TK_RBRACE) {
		ASTNode statement = parseStatement(list, pos);
		if (statement) {
			if (!block->children) block->children = statement;
			else if (lastChild) lastChild->brothers = statement;
			lastChild = statement;
		}
	}

	EXPECT_AND_ADVANCE(list, pos, TK_RBRACE, ERROR_EXPECTED_CLOSING_BRACE, "Missing closing brace '}'");
	return block;
}

/**
 * @brief Converts existing BLOCK_STATEMENT to BLOCK_EXPRESSION for ternary use.
 *
 * @param list Token list
 * @param pos Current position in token list
 * @return BLOCK_EXPRESSION AST node or NULL on error
 */
ASTNode parseBlockExpression(TokenList* list, size_t* pos) {
	ASTNode block = parseBlock(list, pos);
	if (block) block->nodeType = BLOCK_EXPRESSION;
	return block;
}

/**
 * @brief Parses conditional expressions with optional else clause (? syntax).
 *
 * @param list Token list
 * @param pos Current position in token list
 * @param condition Already parsed condition expression
 * @return IF_CONDITIONAL AST node or NULL on error
 */
ASTNode parseTernary(TokenList* list, size_t* pos) {
	EXPECT_TOKEN(list, pos, TK_QUESTION, ERROR_EXPECTED_QUESTION_MARK, "Expected '?'");
	Token* questionToken = &list->tokens[*pos];
	ADVANCE_TOKEN(list, pos);
	ASTNode trueBranch, falseBranch;
	trueBranch = parseExpression(list, pos, PREC_NONE);

	if (!trueBranch) {
		reportError(ERROR_TERNARY_INVALID_CONDITION, createErrorContextFromParser(list, pos), NULL);
		return NULL;
	}
	if(list->tokens[*pos].type != TK_COLON){
		reportError(ERROR_EXPECTED_COLON, createErrorContextFromParser(list, pos), NULL);
	}
	ADVANCE_TOKEN(list, pos);
	falseBranch = parseExpression(list, pos, PREC_NONE);

	ASTNode conditionalNode, trueBranchWrap, falseBranchWrap;
	CREATE_NODE_OR_FAIL(conditionalNode, questionToken, TERNARY_CONDITIONAL, list, pos);
	CREATE_NODE_OR_FAIL(trueBranchWrap, NULL, TERNARY_IF_EXPR, list, pos);
	CREATE_NODE_OR_FAIL(falseBranchWrap, NULL, TERNARY_ELSE_EXPR, list, pos);

	trueBranchWrap->children = trueBranch;
	conditionalNode->children = trueBranchWrap; //temporary children to send trueBranch outside along with conditional Node, this should be Condition children
	falseBranchWrap->children = falseBranch;
	trueBranchWrap->brothers = falseBranchWrap;

	return conditionalNode;
}

ASTNode parseIf(TokenList *list, size_t* pos){
	Token *ifToken = &list->tokens[*pos];
	ADVANCE_TOKEN(list, pos);

	ASTNode falseBranch = NULL;
	ASTNode conditionalNode, condition, trueBranchWrap, trueBranch, falseBranchWrap;
	condition = parseExpression(list, pos, PREC_NONE);
	EXPECT_TOKEN(list, pos, TK_LBRACE, ERROR_EXPECTED_OPENING_BRACE, "Expected '{' after if condition");
	trueBranch = parseBlock(list, pos);

	if(list->tokens[*pos].type == TK_ELSE){
		ADVANCE_TOKEN(list, pos);
		PARSE_OR_CLEANUP(falseBranch, parseBlock(list, pos), condition, trueBranch);
	}

	CREATE_NODE_OR_FAIL(conditionalNode, ifToken, IF_CONDITIONAL, list, pos);
	CREATE_NODE_OR_FAIL(trueBranchWrap, NULL, IF_TRUE_BRANCH, list, pos);

	conditionalNode->children = condition;
	trueBranchWrap->children = trueBranch;
	condition->brothers = trueBranchWrap;

	
	if(falseBranch){
		CREATE_NODE_OR_FAIL(falseBranchWrap, NULL, ELSE_BRANCH, list, pos);
		falseBranchWrap->children = falseBranch;
		trueBranchWrap->brothers = falseBranchWrap;
	}

	return conditionalNode;
}

/**
 * @brief Parses while loop statements using @ syntax.
 *
 * @param list Token list
 * @param pos Current position in token list
 * @return LOOP_STATEMENT AST node or NULL on error
 */
ASTNode parseLoop(TokenList* list, size_t* pos) {
	if (*pos >= list->count) return NULL;
	Token* loopToken = &list->tokens[*pos];
	ADVANCE_TOKEN(list, pos);

	ASTNode condition, loopBody, loopNode;
	PARSE_OR_FAIL(condition, parseExpression(list, pos, PREC_NONE));
	EXPECT_TOKEN(list, pos, TK_LBRACE, ERROR_EXPECTED_OPENING_BRACE, "Expected '{' after loop condition");
	PARSE_OR_FAIL(loopBody, parseBlock(list, pos));
	CREATE_NODE_OR_FAIL(loopNode, loopToken, LOOP_STATEMENT, list, pos);

	loopNode->children = condition;
	condition->brothers = loopBody;
	return loopNode;
}

/**
 * @brief Parses a single parameter in a function declaration.
 *
 * @param list Token list
 * @param pos Current position in token list
 * @return PARAMETER AST node or NULL on error
 */
ASTNode parseParameter(TokenList* list, size_t* pos) {
	if (*pos >= list->count) return NULL;
	Token* token = &list->tokens[*pos];

	if (detectLitType(token, list, pos) != VARIABLE) {
		reportError(ERROR_INVALID_EXPRESSION,createErrorContextFromParser(list, pos), "Expected parameter name");
		return NULL;
	}

	ASTNode paramNode, typeNode, typeRefWrap;
	CREATE_NODE_OR_FAIL(paramNode, token, PARAMETER, list, pos);
	ADVANCE_TOKEN(list, pos);

	EXPECT_AND_ADVANCE(list, pos, TK_COLON, ERROR_EXPECTED_COLON, "Expected ':' after parameter name");
	if (*pos >= list->count || !isTypeToken(list->tokens[*pos].type)) {
		reportError(ERROR_INVALID_EXPRESSION,createErrorContextFromParser(list, pos), "Expected type after ':'");
		freeAST(paramNode);
		return NULL;
	}

	Token* typeToken = &list->tokens[*pos];
	ADVANCE_TOKEN(list, pos);
	CREATE_NODE_OR_FAIL(typeRefWrap, NULL, TYPE_REF, list, pos);
	CREATE_NODE_OR_FAIL(typeNode, NULL, getDecType(typeToken->type), list, pos);
	typeRefWrap->children = typeNode;
	paramNode->children = typeRefWrap;
	return paramNode;
}

/**
 * @brief Wrapper for parsing arguments in function calls.
 *
 * @param list Token list
 * @param pos Current position in token list
 * @return Parsed expression or NULL on error
 */
ASTNode parseArg(TokenList* list, size_t* pos) {
	return parseExpression(list, pos, PREC_NONE);
}

/**
 * @brief Parses comma-separated lists (parameters or arguments).
 *
 * @param list Token list
 * @param pos Current position in token list
 * @param listType Type of list node to create
 * @param parseElement Function to parse individual elements
 * @return List AST node or NULL on error
 */
ASTNode parseCommaSeparatedLists(TokenList* list, size_t* pos, NodeTypes listType,
								 ASTNode (*parseElement)(TokenList*, size_t*)) {
	EXPECT_AND_ADVANCE(list, pos, TK_LPAREN, ERROR_EXPECTED_OPENING_PAREN, "Expected '('");

	ASTNode listNode;
	CREATE_NODE_OR_FAIL(listNode, NULL, listType, list, pos);

	ASTNode last = NULL;
	while (*pos < list->count && list->tokens[*pos].type != TK_RPAREN) {
		ASTNode elem;
		PARSE_OR_CLEANUP(elem, parseElement(list, pos), listNode);

		if (!listNode->children) listNode->children = elem;
		else last->brothers = elem;
		last = elem;

		if (*pos < list->count && list->tokens[*pos].type == TK_COMMA) {
			ADVANCE_TOKEN(list, pos);
		} else if (list->tokens[*pos].type != TK_RPAREN) {
			reportError(ERROR_INVALID_EXPRESSION,createErrorContextFromParser(list, pos), "Expected ',' or ')'");
			freeAST(listNode);
			return NULL;
		}
	}

	EXPECT_AND_ADVANCE(list, pos, TK_RPAREN, ERROR_EXPECTED_CLOSING_PAREN, "Expected ')'");
	return listNode;
}

/**
 * @brief Parses function return type declaration.
 *
 * @param list Token list
 * @param pos Current position in token list
 * @return RETURN_TYPE AST node or NULL on error
 */
ASTNode parseReturnType(TokenList* list, size_t* pos) {
	EXPECT_AND_ADVANCE(list, pos, TK_ARROW, ERROR_EXPECTED_ARROW, "Expected '->'");

	if (*pos >= list->count || !isTypeToken(list->tokens[*pos].type)) {
		reportError(ERROR_INVALID_EXPRESSION,createErrorContextFromParser(list, pos), "Expected type after '->'");
		return NULL;
	}

	Token* typeToken = &list->tokens[*pos];
	NodeTypes returnType = getDecType(typeToken->type);
	ADVANCE_TOKEN(list, pos);

	ASTNode returnTypeNode, typeNode;
	CREATE_NODE_OR_FAIL(returnTypeNode, NULL, RETURN_TYPE, list, pos);

	if (returnType != null_NODE) {
		CREATE_NODE_OR_FAIL(typeNode, NULL, returnType, list, pos);
		returnTypeNode->children = typeNode;
	}

	return returnTypeNode;
}

/**
 * @brief Parses function call expressions.
 *
 * @param list Token list
 * @param pos Current position in token list
 * @return FUNCTION_CALL AST node or NULL on error
 */
ASTNode parseFunctionCall(TokenList* list, size_t* pos, Token * tok) {
	EXPECT_TOKEN(list, pos, TK_LPAREN, ERROR_EXPECTED_OPENING_PAREN, "Expected '(' for function call");

	ASTNode callNode, argList;
	CREATE_NODE_OR_FAIL(callNode, tok, FUNCTION_CALL, list, pos);

	PARSE_OR_CLEANUP(argList, parseCommaSeparatedLists(list, pos, ARGUMENT_LIST, parseArg),
					 callNode);

	callNode->children = argList;
	return callNode;
}

/**
 * @brief Parses return statements.
 *
 * @param list Token list
 * @param pos Current position in token list
 * @return RETURN_STATEMENT AST node or NULL on error
 */
ASTNode parseReturnStatement(TokenList* list, size_t* pos) {
	EXPECT_TOKEN(list, pos, TK_RETURN, ERROR_EXPECTED_RETURN, "Expected 'return' keyword");
	Token* returnToken = &list->tokens[*pos];
	ADVANCE_TOKEN(list, pos);

	ASTNode returnNode;
	CREATE_NODE_OR_FAIL(returnNode, returnToken, RETURN_STATEMENT, list, pos);

	if (*pos < list->count && list->tokens[*pos].type != TK_SEMI) {
		returnNode->children = parseExpression(list, pos, PREC_NONE);
	}

	EXPECT_AND_ADVANCE(list, pos, TK_SEMI, ERROR_EXPECTED_SEMICOLON, "Expected ';' after return statement");
	return returnNode;
}

/**
 * @brief Parses function definitions.
 *
 * @param list Token list
 * @param pos Current position in token list
 * @return FUNCTION_DEFINITION AST node or NULL on error
 */
ASTNode parseFunction(TokenList* list, size_t* pos) {
	EXPECT_TOKEN(list, pos, TK_FN, ERROR_EXPECTED_FN, "Expected 'fn'");
	ADVANCE_TOKEN(list, pos);

	if (*pos >= list->count || detectLitType(&list->tokens[*pos], list, pos) != VARIABLE) {
		reportError(ERROR_INVALID_EXPRESSION,createErrorContextFromParser(list, pos), "Expected function name after 'fn'");
		return NULL;
	}
	Token * name = &list->tokens[*pos];
	ASTNode functionNode;
	CREATE_NODE_OR_FAIL(functionNode, name, FUNCTION_DEFINITION, list, pos);
	ADVANCE_TOKEN(list, pos);

	ASTNode paramList, returnType, body;
	PARSE_OR_CLEANUP(paramList, parseCommaSeparatedLists(list, pos, PARAMETER_LIST, parseParameter),
					 functionNode);
	PARSE_OR_CLEANUP(returnType, parseReturnType(list, pos), functionNode, paramList);
	EXPECT_TOKEN(list, pos, TK_LBRACE, ERROR_EXPECTED_OPENING_BRACE, "Expected '{' for function body");
	PARSE_OR_CLEANUP(body, parseBlock(list, pos), functionNode, paramList, returnType);

	functionNode->children = paramList;
	paramList->brothers = returnType;
	returnType->brothers = body;

	return functionNode;
}

NodeTypes getTypeNodeFromToken(TokenType type) {
	switch (type) {
		case TK_INT: return REF_INT;
		case TK_STRING: return REF_STRING;
		case TK_FLOAT: return REF_FLOAT;
		case TK_BOOL: return REF_BOOL;
		case TK_VOID: return REF_VOID;
		case TK_DOUBLE: return REF_DOUBLE;
		default: return null_NODE; 
	}
}

ASTNode parseStructField(TokenList * list, size_t* pos) {
	Token * name = &list->tokens[*pos];
	if (detectLitType(name, list, pos) != VARIABLE) {
		reportError(ERROR_INVALID_EXPRESSION, createErrorContextFromParser(list, pos), "Expected field name");
		return NULL;
	}
	ASTNode fieldNode, typeNode;
	CREATE_NODE_OR_FAIL(fieldNode, name, STRUCT_FIELD, list, pos);
	ADVANCE_TOKEN(list, pos);
	EXPECT_AND_ADVANCE(list, pos, TK_COLON, ERROR_EXPECTED_COLON, "Expected ':' after field name");
	if (!isTypeToken(list->tokens[*pos].type)) {
		reportError(ERROR_INVALID_EXPRESSION, createErrorContextFromParser(list, pos), "Expected type after ':'");
		freeAST(fieldNode);
		return NULL;
	}
	Token * typeTok = &list->tokens[*pos];
	ADVANCE_TOKEN(list, pos);
	CREATE_NODE_OR_FAIL(typeNode, typeTok, getTypeNodeFromToken(typeTok->type), list, pos);
	fieldNode->children = typeNode;
	return fieldNode;
}

ASTNode parseStruct(TokenList *list, size_t *pos) {
	EXPECT_TOKEN(list, pos, TK_STRUCT, ERROR_INVALID_EXPRESSION, "expected struct");
	ADVANCE_TOKEN(list, pos);
	Token * name = &list->tokens[*pos];
	if (detectLitType(name, list, pos) != VARIABLE) {
		reportError(ERROR_INVALID_EXPRESSION, createErrorContextFromParser(list, pos), "Expected name for struct");
		return NULL;
	}
	ASTNode structNode;
	CREATE_NODE_OR_FAIL(structNode, name, STRUCT_DEFINITION, list, pos);
	ADVANCE_TOKEN(list, pos);
	EXPECT_AND_ADVANCE(list, pos, TK_LBRACE, ERROR_EXPECTED_OPENING_BRACE, "Expected '{'");
	ASTNode fieldList;
	CREATE_NODE_OR_FAIL(fieldList, NULL, STRUCT_FIELD_LIST, list, pos);
	ASTNode last = NULL;
	while (list->tokens[*pos].type != TK_RBRACE) {
		ASTNode field;
		PARSE_OR_CLEANUP(field, parseStructField(list, pos), structNode, fieldList);

		if (!fieldList->children) fieldList->children = field;
		else if (last) last->brothers = field;
		last = field;

		// ';' are optional inside structs
		if (*pos < list->count && list->tokens[*pos].type == TK_SEMI) {
			ADVANCE_TOKEN(list, pos);
		}
	}

	EXPECT_AND_ADVANCE(list, pos, TK_RBRACE, ERROR_EXPECTED_CLOSING_BRACE, "Expected '}' to close struct");
	EXPECT_AND_ADVANCE(list, pos, TK_SEMI, ERROR_EXPECTED_SEMICOLON, "Expected ';' after struct definition");
	structNode->children = fieldList;
	return structNode;
}

/**
 * @brief Parses array type declarations like int[5]
 *
 * @param list Token list
 * @param pos Current position in token list
 * @param tokType Type token (int, float, etc.)
 * @param varName Variable name token
 * @return ARRAY_VARIABLE_DEFINITION node or NULL on error
 */
ASTNode parseArrayDec(TokenList *list, size_t *pos, Token *tokType, Token *varName) {
	if (*pos >= list->count) return NULL;
	
	EXPECT_AND_ADVANCE(list, pos, TK_LBRACKET, ERROR_EXPECTED_OPENING_BRACKET, "Expected '[' for array size");

    Token *sizeToken = &list->tokens[*pos];
    NodeTypes sizeType = detectLitType(sizeToken, list, pos);

    // Accept both integer literals and variables (validated later in typechecker)
    if (sizeType != REF_INT && sizeType != VARIABLE) {
        reportError(ERROR_INVALID_EXPRESSION, createErrorContextFromParser(list, pos),
                    "Array size must be an integer literal or variable");
        return NULL;
    }

        ASTNode sizeNode = createValNode(sizeToken, list, pos);
	if (!sizeNode) return NULL;
	ADVANCE_TOKEN(list, pos);
	
	EXPECT_AND_ADVANCE(list, pos, TK_RBRACKET, ERROR_EXPECTED_CLOSING_BRACKET, "Expected ']' after array size");
	
	ASTNode arrayDefNode;
	CREATE_NODE_OR_FAIL(arrayDefNode, varName, ARRAY_VARIABLE_DEFINITION, list, pos);
	
	NodeTypes elementType = getDecType(tokType->type);
	ASTNode typeRefNode, typeNode;
	CREATE_NODE_OR_FAIL(typeRefNode, NULL, TYPE_REF, list, pos);
	CREATE_NODE_OR_FAIL(typeNode, NULL, elementType, list, pos);
	typeRefNode->children = typeNode;
	
	arrayDefNode->children = typeRefNode;
	typeRefNode->brothers = sizeNode;
	
	return arrayDefNode;
}

/**
 * @brief Parses array literals like [1, 2, 3]
 *
 * @param list Token list
 * @param pos Current position in token list
 * @return ARRAY_LIT node or NULL on error
 */
ASTNode parseArrLit(TokenList *list, size_t *pos) {
	if (*pos >= list->count) return NULL;
	
	Token *startToken = &list->tokens[*pos];
	EXPECT_AND_ADVANCE(list, pos, TK_LBRACKET, ERROR_EXPECTED_OPENING_BRACKET, "Expected '[' for array literal");
	
	ASTNode arrayLitNode;
	CREATE_NODE_OR_FAIL(arrayLitNode, startToken, ARRAY_LIT, list, pos);
	
	// empty array
	if (*pos < list->count && list->tokens[*pos].type == TK_RBRACKET) {
		ADVANCE_TOKEN(list, pos);
		return arrayLitNode;
	}
	
	ASTNode lastElement = NULL;
	while (*pos < list->count && list->tokens[*pos].type != TK_RBRACKET) {
		ASTNode element = parseExpression(list, pos, PREC_NONE);
		if (!element) {
			freeAST(arrayLitNode);
			return NULL;
		}
		
		if (!arrayLitNode->children) {
			arrayLitNode->children = element;
			lastElement = element;
		} else {
			lastElement->brothers = element;
			lastElement = element;
		}
		
		if (*pos < list->count && list->tokens[*pos].type == TK_COMMA) {
			ADVANCE_TOKEN(list, pos);
		} else if (*pos < list->count && list->tokens[*pos].type != TK_RBRACKET) {
			reportError(ERROR_EXPECTED_COMMA, createErrorContextFromParser(list, pos),
					   "Expected ',' or ']' in array literal");
			freeAST(arrayLitNode);
			return NULL;
		}
	}
	
	EXPECT_AND_ADVANCE(list, pos, TK_RBRACKET, ERROR_EXPECTED_CLOSING_BRACKET, "Expected ']' after array literal");
	return arrayLitNode;
}

/**
 * @brief Parses array access expressions like arr[index]
 *
 * @param list Token list
 * @param pos Current position in token list
 * @param arrNode The array variable node
 * @return ARRAY_ACCESS node or NULL on error
 */
ASTNode parseArrayAccess(TokenList *list, size_t *pos, ASTNode arrNode) {
	if (*pos >= list->count || !arrNode) return NULL;
	
	EXPECT_AND_ADVANCE(list, pos, TK_LBRACKET, ERROR_EXPECTED_OPENING_BRACKET, "Expected '['.");
	
	ASTNode indexExpr = parseExpression(list, pos, PREC_NONE);
	if (!indexExpr) {
		freeAST(arrNode);
		return NULL;
	}
	
	EXPECT_AND_ADVANCE(list, pos, TK_RBRACKET, ERROR_EXPECTED_CLOSING_BRACKET, "Expected ']' after array index");
	
	ASTNode accessNode;
	CREATE_NODE_OR_FAIL(accessNode, NULL, ARRAY_ACCESS, list, pos);
	accessNode->children = arrNode;
	arrNode->brothers = indexExpr;

	return accessNode;
}

/**
 * @brief Parses variable declarations.
 *
 * @param list Token list
 * @param pos Current position in token list
 * @param decType Declaration type
 * @return Declaration AST node or NULL on error
 */
ASTNode parseDeclaration(TokenList* list, size_t* pos) {
	int isConst = list->tokens[*pos].type == TK_CONST;
	Token *keyTok = &list->tokens[*pos];
	ADVANCE_TOKEN(list, pos);

	Token *varName = &list->tokens[*pos];
	if(detectLitType(varName, list, pos) != VARIABLE){
		reportError(ERROR_INVALID_EXPRESSION, createErrorContextFromParser(list, pos), 
                   "Expected identifier after const/let");
        return NULL;
	}
	ADVANCE_TOKEN(list, pos);
	EXPECT_AND_ADVANCE(list, pos, TK_COLON, ERROR_EXPECTED_COLON, "Expected ':' after identifier");
	if(*pos >= list->count || !isTypeToken(list->tokens[*pos].type)){
		reportError(ERROR_INVALID_EXPRESSION, createErrorContextFromParser(list, pos), 
                   "Expected type after ':'");
        return NULL;
	}

	Token *typeToken = &list->tokens[*pos];
	NodeTypes varRefType = getDecType(typeToken->type);

	if (varRefType == null_NODE) {
        reportError(ERROR_INVALID_EXPRESSION, createErrorContextFromParser(list, pos),
                   "Invalid type in declaration");
        return NULL;
    }
	
	ADVANCE_TOKEN(list, pos);

	// Check if this is an array declaration
	int isArray = (*pos < list->count && list->tokens[*pos].type == TK_LBRACKET);

	ASTNode mutWrapNode;
	CREATE_NODE_OR_FAIL(mutWrapNode, keyTok, isConst ? CONST_DEC : LET_DEC, list, pos);

	ASTNode varDefNode;
	if (isArray) {
		// Parse array type declaration
		varDefNode = parseArrayDec(list, pos, typeToken, varName);
		if (!varDefNode) {
			freeAST(mutWrapNode);
			return NULL;
		}
	} else {
		// Regular variable definition
		CREATE_NODE_OR_FAIL(varDefNode, varName, VAR_DEFINITION, list, pos);
		
		ASTNode typeRefWrapNode, typeNode;
		CREATE_NODE_OR_FAIL(typeRefWrapNode, NULL, TYPE_REF, list, pos);
		CREATE_NODE_OR_FAIL(typeNode, NULL, varRefType, list, pos);
		varDefNode->children=typeRefWrapNode;
		typeRefWrapNode->children = typeNode;
	}
    
    mutWrapNode->children = varDefNode;
    
    if (*pos < list->count && list->tokens[*pos].type == TK_ASSIGN) {
        ADVANCE_TOKEN(list, pos);
        ASTNode initExpr, valueWrap;
		CREATE_NODE_OR_FAIL(valueWrap, NULL, VALUE, list, pos);
        PARSE_OR_CLEANUP(initExpr, parseExpression(list, pos, PREC_NONE), 
                        mutWrapNode, varDefNode);
        valueWrap->children = initExpr;
        
        // Attach value to appropriate node
        if (isArray) {
            // For arrays, attach to the array def node
            if (varDefNode->children && varDefNode->children->brothers) {
                // Navigate to end of brothers (after type and size)
                ASTNode lastBrother = varDefNode->children->brothers;
                while (lastBrother->brothers) lastBrother = lastBrother->brothers;
                lastBrother->brothers = valueWrap;
            } else if (varDefNode->children) {
                varDefNode->children->brothers = valueWrap;
            }
        } else {
            // For regular variables, attach to type ref
            ASTNode typeRefWrapNode = varDefNode->children;
            if (typeRefWrapNode) {
                typeRefWrapNode->brothers = valueWrap;
            }
        }
    } else if (isConst) {
        reportError(ERROR_INVALID_EXPRESSION, createErrorContextFromParser(list, pos),
                   "const declarations must have an initializer");
        freeAST(mutWrapNode);
        return NULL;
    }
    
    EXPECT_AND_ADVANCE(list, pos, TK_SEMI, ERROR_EXPECTED_SEMICOLON, 
                      "Expected ';' after declaration");
    
    return mutWrapNode;

}

/**
 * @brief Parses expression statements.
 *
 * @param list Token list
 * @param pos Current position in token list
 * @return Expression AST node or NULL on error
 */
ASTNode parseExpressionStatement(TokenList* list, size_t* pos) {
	ASTNode expressionNode;
	PARSE_OR_FAIL(expressionNode, parseExpression(list, pos, PREC_NONE));
	EXPECT_AND_ADVANCE(list, pos, TK_SEMI,ERROR_EXPECTED_SEMICOLON, "Expected ';'");
	return expressionNode;
}

ASTNode parseStructVarDec(TokenList* list, size_t* pos) {
	Token * type = &list->tokens[*pos];
	ADVANCE_TOKEN(list, pos);
	Token * varTok = &list->tokens[*pos];
	ADVANCE_TOKEN(list, pos);

	ASTNode structVarNode;
	CREATE_NODE_OR_FAIL(structVarNode, varTok, STRUCT_VARIABLE_DEFINITION, list, pos);
	ASTNode typeRefNode;
	CREATE_NODE_OR_FAIL(typeRefNode, type, REF_CUSTOM, list, pos);
	structVarNode->children = typeRefNode;
	if (*pos < list->count && list->tokens[*pos].type == TK_ASSIGN) {
		ADVANCE_TOKEN(list, pos);
		ASTNode initExpr;
		PARSE_OR_CLEANUP(initExpr, parseExpression(list, pos, PREC_NONE), structVarNode);
		typeRefNode->brothers = initExpr;
	}

	EXPECT_AND_ADVANCE(list, pos, TK_SEMI, ERROR_EXPECTED_SEMICOLON, "Expected ';'");
	return structVarNode;
}

/**
 * @brief Parses individual statements.
 *
 * Main statement parsing function that handles various statement types.
 *
 * @param list Token list
 * @param pos Current position in token list
 * @return AST node for the parsed statement or NULL for empty/invalid statements
 */
ASTNode parseStatement(TokenList* list, size_t* pos) {
	if (*pos >= list->count) return NULL;

	Token* currentToken = &list->tokens[*pos];

	if (currentToken->type == TK_SEMI) {
		ADVANCE_TOKEN(list, pos);
		return NULL;
	}

	// Check for statement handlers
	for (int i = 0; statementHandlers[i].token != TK_NULL; i++) {
		if (currentToken->type == statementHandlers[i].token) {
			return statementHandlers[i].handler(list, pos);
		}
	}

	// Check for const/let declarations
    if (currentToken->type == TK_CONST || currentToken->type == TK_LET) {
        return parseDeclaration(list, pos);
    }

	if (currentToken->type == TK_LIT && list->tokens[*pos+1].type == TK_LIT) {
		return parseStructVarDec(list, pos);
	}

	// Default to expression statement
	return parseExpressionStatement(list, pos);
}

ASTContext * buildASTContextFromTokenList(TokenList* list) {
	ASTContext * astContext = malloc(sizeof(struct ASTContext));
	astContext->buffer = list->buffer;
	astContext->filename = list->filename;
	return astContext;
}

/**
 * @brief Main AST generation function - parses complete token stream.
 *
 * Creates the root PROGRAM node and parses all statements in the token stream.
 *
 * @param tokenList Token list from lexer
 * @return Root PROGRAM node containing all parsed statements
 */
ASTContext * ASTGenerator(TokenList* tokenList) {
	if (!tokenList || tokenList->count == 0) return NULL;
	ASTContext * astContext = buildASTContextFromTokenList(tokenList);
	ASTNode programNode;
	size_t pos = 0;
	CREATE_NODE_OR_FAIL(programNode, NULL, PROGRAM, tokenList, &pos);
	astContext->root = programNode;

	ASTNode lastStatement = NULL;
	size_t lastPos = (size_t)-1;

	while (pos < tokenList->count) {
		if (pos == lastPos) {
			// Parser is stuck - skip token and continue
			reportError(ERROR_INVALID_EXPRESSION,createErrorContextFromParser(tokenList, &pos), "Parser stuck - skipping token");
			pos++;
			continue;
		}
		ASTNode currentStatement = parseStatement(tokenList, &pos);
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

	return astContext;
}

/**
 * @brief Recursively prints AST tree structure with visual formatting.
 *
 * @param node Current AST node to print
 * @param prefix String prefix for indentation and tree lines
 * @param isLast Flag indicating if this is the last sibling
 */
void printASTTree(ASTNode node, char* prefix, int isLast) {
	if (node == NULL) return;

	const char* nodeTypeStr = getNodeTypeName(node->nodeType);

	printf("%s%s%s", prefix, isLast ? "|___ " : "|-- ", nodeTypeStr);
	if (node->start && node->length > 0) {
		char* str = extractText(node->start,node->length);
		if (str) {
			printf(": %s", str);
			free(str);
		}
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
 * @param node Root AST node to print (typically PROGRAM node)
 * @param depth Indentation depth (maintained for compatibility, unused)
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
 * @param node Root node to free (can be NULL)
 */
void freeAST(ASTNode node) {
	if (node == NULL) return;

	freeAST(node->children);
	freeAST(node->brothers);
	free(node);
}
void freeASTContext(ASTContext* ctx) {
	if (ctx) {
		if (ctx->root) freeAST(ctx->root);
		free(ctx);
	}
}
