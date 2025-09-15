/**
* @file parser.h
 * @brief Parser interface and Abstract Syntax Tree definitions for the C compiler.
 *
 * Defines the AST node types, data structures, and function prototypes for
 * parsing tokens into an Abstract Syntax Tree. Implements operator precedence
 * parsing using a Pratt parser approach for expression handling.
 */

#ifndef PARSER_H
#define PARSER_H

#include "errorHandling.h"
#include "../lexer/lexer.h"

char *tokenToString(const Token *token);
ErrorContext *createErrorContextFromParser(TokenList *list, size_t * pos) ;
const char *getTokenTypeName(TokenType type);
const char *getCurrentTokenName(TokenList *list, size_t pos);

#define ADVANCE_TOKEN(list, pos) do { if (*(pos) < (list)->count) (*(pos))++; } while(0)

#define EXPECT_TOKEN(list, pos, expected_type, err_msg) \
    do { \
        if (*(pos) >= (list)->count || (list)->tokens[*(pos)].type != (expected_type)) { \
            reportError(ERROR_INVALID_EXPRESSION,createErrorContextFromParser(list, pos), err_msg ? err_msg : "Unexpected token"); \
            return NULL; \
        } \
    } while(0)

#define EXPECT_AND_ADVANCE(list, pos, expected_type, err_msg) \
    do { \
        EXPECT_TOKEN(list, pos, expected_type, err_msg); \
        ADVANCE_TOKEN(list, pos); \
    } while(0)

#define CREATE_NODE_OR_FAIL(var, token, type, list, pos) \
    do { \
        var = createNode(token, type, list, pos); \
        if (!var) return NULL; \
    } while(0)


#define PARSE_OR_CLEANUP(var, parse_expr, ...) \
    do { \
        var = (parse_expr); \
        if (!var) { \
            ASTNode _cleanup_nodes[] = {__VA_ARGS__}; \
            for (size_t _i = 0; _i < sizeof(_cleanup_nodes)/sizeof(_cleanup_nodes[0]); _i++) { \
                if (_cleanup_nodes[_i]) freeAST(_cleanup_nodes[_i]); \
            } \
            return NULL; \
        } \
    } while(0)

/**
 * @brief Enumeration of all Abstract Syntax Tree node types.
 *
 * Organized by category for better maintainability:
 * - Program structure nodes
 * - Variable declaration nodes
 * - Literal value nodes
 * - Expression nodes (operators, assignments)
 * - Statement nodes (blocks, control flow)
 *
 * Each node type represents a distinct syntactic construct in the language.
 */
typedef enum {
    null_NODE,
    PROGRAM,
    STRING_VARIABLE_DEFINITION,
    INT_VARIABLE_DEFINITION,
    FLOAT_VARIABLE_DEFINITION,
    BOOL_VARIABLE_DEFINITION,
    STRING_LIT,
    INT_LIT,
    FLOAT_LIT,
    BOOL_LIT,
    VARIABLE,
    ASSIGNMENT,
    COMPOUND_ADD_ASSIGN,
    COMPOUND_SUB_ASSIGN,
    COMPOUND_MUL_ASSIGN,
    COMPOUND_DIV_ASSIGN,
    ADD_OP,
    SUB_OP,
    MUL_OP,
    DIV_OP,
    MOD_OP,
    UNARY_MINUS_OP,
    UNARY_PLUS_OP,
    PRE_INCREMENT,
    PRE_DECREMENT,
    POST_INCREMENT,
    POST_DECREMENT,
    LOGIC_AND,
    LOGIC_OR,
    LOGIC_NOT,
    EQUAL_OP,
    NOT_EQUAL_OP,
    LESS_THAN_OP,
    GREATER_THAN_OP,
    LESS_EQUAL_OP,
    GREATER_EQUAL_OP,
    BLOCK_STATEMENT,
    IF_CONDITIONAL,
    IF_TRUE_BRANCH,
    ELSE_BRANCH,
    BLOCK_EXPRESSION,
    LOOP_STATEMENT,
    FUNCTION_DEFINITION,
    FUNCTION_CALL,
    PARAMETER_LIST,
    PARAMETER,
    ARGUMENT_LIST,
    RETURN_STATEMENT,
    RETURN_TYPE,
} NodeTypes;


typedef struct {
    NodeTypes nodeType;
    const char *displayName;
} NodeTypeMap;

static const NodeTypeMap nodeTypeMapping[] = {
    {PROGRAM, "PROGRAM"},
    {STRING_VARIABLE_DEFINITION, "STRING_VAR_DEF"},
    {INT_VARIABLE_DEFINITION, "INT_VAR_DEF"},
    {FLOAT_VARIABLE_DEFINITION, "FLOAT_VAR_DEF"},
    {BOOL_VARIABLE_DEFINITION, "BOOL_VAR_DEF"},
    {STRING_LIT, "STRING_LIT"},
    {INT_LIT, "INT_LIT"},
    {FLOAT_LIT, "FLOAT_LIT"},
    {BOOL_LIT, "BOOL_LIT"},
    {VARIABLE, "VARIABLE"},
    {ASSIGNMENT, "ASSIGNMENT"},
    {COMPOUND_ADD_ASSIGN, "COMPOUND_ADD_ASSIGN"},
    {COMPOUND_SUB_ASSIGN, "COMPOUND_SUB_ASSIGN"},
    {COMPOUND_MUL_ASSIGN, "COMPOUND_MULT_ASSIGN"},
    {COMPOUND_DIV_ASSIGN, "COMPOUND_DIV_ASSIGN"},
    {ADD_OP, "ADD_OP"},
    {SUB_OP, "SUB_OP"},
    {MUL_OP, "MUL_OP"},
    {DIV_OP, "DIV_OP"},
    {MOD_OP, "MOD_OP"},
    {UNARY_MINUS_OP, "UNARY_MINUS_OP"},
    {UNARY_PLUS_OP, "UNARY_PLUS_OP"},
    {PRE_INCREMENT, "PRE_INCREMENT"},
    {PRE_DECREMENT, "PRE_DECREMENT"},
    {POST_INCREMENT, "POST_INCREMENT"},
    {POST_DECREMENT, "POST_DECREMENT"},
    {LOGIC_AND, "LOGIC_AND"},
    {LOGIC_OR, "LOGIC_OR"},
    {LOGIC_NOT, "LOGIC_NOT"},
    {EQUAL_OP, "EQUAL_OP"},
    {NOT_EQUAL_OP, "NOT_EQUAL_OP"},
    {LESS_THAN_OP, "LESS_THAN_OP"},
    {GREATER_THAN_OP, "GREATER_THAN_OP"},
    {LESS_EQUAL_OP, "LESS_EQUAL_OP"},
    {GREATER_EQUAL_OP, "GREATER_EQUAL_OP"},
    {BLOCK_STATEMENT, "BLOCK_STATEMENT"},
    {IF_CONDITIONAL, "IF_CONDITIONAL"},
    {IF_TRUE_BRANCH, "IF_TRUE_BRANCH"},
    {ELSE_BRANCH, "ELSE_BRANCH"},
    {BLOCK_EXPRESSION, "BLOCK_EXPRESSION"},
    {LOOP_STATEMENT, "LOOP_STATEMENT"},
    {FUNCTION_DEFINITION, "FUNCTION_DEFINITION"},
    {FUNCTION_CALL, "FUNCTION_CALL"},
    {PARAMETER_LIST, "PARAMETER_LIST"},
    {PARAMETER, "PARAMETER"},
    {ARGUMENT_LIST, "ARGUMENT_LIST"},
    {RETURN_STATEMENT, "RETURN_STATEMENT"},
    {RETURN_TYPE, "RETURN_TYPE"},
    {null_NODE, NULL} // Sentinel - must be last
};

struct ASTNode {
    char *value;
    NodeTypes nodeType;
    int line;
    int column;
    struct ASTNode *children;
    struct ASTNode *brothers;
};

typedef struct ASTNode *ASTNode;

typedef ASTNode (*ParseFunc)(TokenList*, size_t*);

typedef struct {
    TokenType token;
    ParseFunc handler;
} StatementHandler;

extern const StatementHandler statementHandlers[];

/**
 * @brief Static mapping table for type definitions.
 *
 * Terminated with TK_NULL entry for easy iteration.
 * Used by getDecType() to convert tokens to AST node types.
 */
typedef struct {
    TokenType TkType;
    NodeTypes type;
} TypeDefMap;

static const TypeDefMap TypeDefs[] = {
    {TK_INT, INT_VARIABLE_DEFINITION},
    {TK_STRING, STRING_VARIABLE_DEFINITION},
    {TK_FLOAT, FLOAT_VARIABLE_DEFINITION},
    {TK_BOOL, BOOL_VARIABLE_DEFINITION},
    {TK_NULL, null_NODE}
};

/**
 * @brief Operator precedence levels for Pratt parser.
 *
 * Higher numeric values indicate higher precedence.
 * Used to resolve operator precedence during expression parsing.
 *
 * Precedence hierarchy (low to high):
 * - PREC_ASSIGN: Assignment operators (=, +=, -=, etc.)
 * - PREC_OR: Logical OR (||)
 * - PREC_AND: Logical AND (&&)
 * - PREC_EQUALITY: Equality operators (==, !=)
 * - PREC_COMPARISON: Comparison operators (<, >, <=, >=)
 * - PREC_TERM: Additive operators (+, -)
 * - PREC_FACTOR: Multiplicative operators (*, /, %)
 * - PREC_UNARY: Unary operators (!, ++, --, unary -)
 */
typedef enum {
    PREC_NONE,
    PREC_ASSIGN,
    PREC_TERNARY,
    PREC_OR,
    PREC_AND,
    PREC_EQUALITY,
    PREC_COMPARISON,
    PREC_TERM,
    PREC_FACTOR,
    PREC_UNARY
} Precedence;

/**
 * @brief Operator information for precedence parsing.
 *
 * Contains all information needed for Pratt parser to handle operators:
 * - Token type recognition
 * - AST node type mapping
 * - Precedence level
 * - Associativity information
 */
typedef struct {
    TokenType token;
    NodeTypes nodeType;
    Precedence precedence;
    int isRightAssociative; // Flag for right-associative operators like '='
} OperatorInfo;

/**
 * @brief Static operator information table.
 *
 * Defines all operators supported by the parser with their precedence
 * and associativity information. Used by getOperatorInfo() during parsing.
 *
 * @note Assignment operators are right-associative (x = y = z evaluates as x = (y = z))
 *       All other operators are left-associative (a + b + c evaluates as (a + b) + c)
 */
static const OperatorInfo operators[] = {
    {TK_ASSIGN, ASSIGNMENT, PREC_ASSIGN, 1},
    {TK_PLUS_ASSIGN, COMPOUND_ADD_ASSIGN, PREC_ASSIGN, 1},
    {TK_MINUS_ASSIGN, COMPOUND_SUB_ASSIGN, PREC_ASSIGN, 1},
    {TK_STAR_ASSIGN, COMPOUND_MUL_ASSIGN, PREC_ASSIGN, 1},
    {TK_SLASH_ASSIGN, COMPOUND_DIV_ASSIGN, PREC_ASSIGN, 1},
    {TK_OR, LOGIC_OR, PREC_OR, 0},
    {TK_AND, LOGIC_AND, PREC_AND, 0},
    {TK_EQ, EQUAL_OP, PREC_EQUALITY, 0},
    {TK_NOT_EQ, NOT_EQUAL_OP, PREC_EQUALITY, 0},
    {TK_LESS, LESS_THAN_OP, PREC_COMPARISON, 0},
    {TK_GREATER, GREATER_THAN_OP, PREC_COMPARISON, 0},
    {TK_LESS_EQ, LESS_EQUAL_OP, PREC_COMPARISON, 0},
    {TK_GREATER_EQ, GREATER_EQUAL_OP, PREC_COMPARISON, 0},
    {TK_PLUS, ADD_OP, PREC_TERM, 0},
    {TK_MINUS, SUB_OP, PREC_TERM, 0},
    {TK_STAR, MUL_OP, PREC_FACTOR, 0},
    {TK_SLASH, DIV_OP, PREC_FACTOR, 0},
    {TK_MOD, MOD_OP, PREC_FACTOR, 0},
    {TK_NULL, null_NODE, PREC_NONE, 0}
};


// helpers
NodeTypes getDecType(TokenType type);
ASTNode createNode(const Token* token, NodeTypes type, TokenList * list, size_t * pos);
const char *getNodeTypeName(NodeTypes nodeType);
ASTNode createValNode(const Token* current_token, TokenList *list, size_t* pos);
const OperatorInfo *getOperatorInfo(TokenType type);
int isTypeToken(TokenType type);
NodeTypes getReturnTypeFromToken(TokenType type);
NodeTypes getUnaryOpType(TokenType t);
NodeTypes detectLitType(const Token* tok, TokenList * list, size_t * pos);

// Parser function declarations
ASTNode parseStatement(TokenList* list, size_t* pos);
ASTNode parseExpression(TokenList* list, size_t* pos, Precedence minPrec);
ASTNode parseUnary(TokenList* list, size_t* pos);
ASTNode parsePrimaryExp(TokenList* list, size_t* pos);
ASTNode parseFunction(TokenList* list, size_t* pos);
ASTNode parseFunctionCall(TokenList* list, size_t* pos, char *functionName);
ASTNode parseReturnStatement(TokenList* list, size_t* pos);
ASTNode parseLoop(TokenList* list, size_t* pos);
ASTNode parseBlock(TokenList* list, size_t* pos);
ASTNode parseBlockExpression(TokenList* list, size_t* pos);
ASTNode parseConditional(TokenList* list, size_t* pos, ASTNode condition);
ASTNode parseParameter(TokenList* list, size_t* pos);
ASTNode parseArg(TokenList* list, size_t* pos);
ASTNode parseCommaSeparatedLists(TokenList* list, size_t* pos, NodeTypes listType,
                                  ASTNode (*parseElement)(TokenList*, size_t*));
ASTNode parseReturnType(TokenList* list, size_t* pos);
ASTNode parseDeclaration(TokenList* list, size_t* pos, NodeTypes decType);
ASTNode parseExpressionStatement(TokenList* list, size_t* pos);

// Public function prototypes
ASTNode ASTGenerator(TokenList* tokenList);
void printAST(ASTNode node, int depth);
void printASTTree(ASTNode node, char *prefix, int isLast);
void freeAST(ASTNode node);

#endif //PARSER_H
