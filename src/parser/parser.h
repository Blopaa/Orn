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

#include "../lexer/lexer.h"

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

/**
 * @brief Static mapping table for type definitions.
 *
 * Terminated with TokenNULL entry for easy iteration.
 * Used by getDecType() to convert tokens to AST node types.
 */
typedef struct {
    TokenType TkType;
    NodeTypes type;
} TypeDefMap;

static const TypeDefMap TypeDefs[] = {
    {TokenIntDefinition, INT_VARIABLE_DEFINITION},
    {TokenStringDefinition, STRING_VARIABLE_DEFINITION},
    {TokenFloatDefinition, FLOAT_VARIABLE_DEFINITION},
    {TokenBoolDefinition, BOOL_VARIABLE_DEFINITION},
    {TokenNULL, null_NODE}
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
    {TokenAssignement, ASSIGNMENT, PREC_ASSIGN, 1},
    {TokenPlusAssign, COMPOUND_ADD_ASSIGN, PREC_ASSIGN, 1},
    {TokenSubAssign, COMPOUND_SUB_ASSIGN, PREC_ASSIGN, 1},
    {TokenMultAssign, COMPOUND_MUL_ASSIGN, PREC_ASSIGN, 1},
    {TokenDivAssign, COMPOUND_DIV_ASSIGN, PREC_ASSIGN, 1},
    {TokenOr, LOGIC_OR, PREC_OR, 0},
    {TokenAnd, LOGIC_AND, PREC_AND, 0},
    {TokenEqual, EQUAL_OP, PREC_EQUALITY, 0},
    {TokenNotEqual, NOT_EQUAL_OP, PREC_EQUALITY, 0},
    {TokenLess, LESS_THAN_OP, PREC_COMPARISON, 0},
    {TokenGreater, GREATER_THAN_OP, PREC_COMPARISON, 0},
    {TokenLessEqual, LESS_EQUAL_OP, PREC_COMPARISON, 0},
    {TokenGreaterEqual, GREATER_EQUAL_OP, PREC_COMPARISON, 0},
    {TokenSum, ADD_OP, PREC_TERM, 0},
    {TokenSub, SUB_OP, PREC_TERM, 0},
    {TokenMult, MUL_OP, PREC_FACTOR, 0},
    {TokenDiv, DIV_OP, PREC_FACTOR, 0},
    {TokenMod, MOD_OP, PREC_FACTOR, 0},
    {TokenNULL, null_NODE, PREC_NONE, 0}
};


// --- HELPER FUNCTION DECLARATIONS ---
NodeTypes getDecType(TokenType type);

ASTNode createNode(Token token, NodeTypes type);

const char *getNodeTypeName(NodeTypes nodeType);

// --- VALIDATION FUNCTION DECLARATIONS ---
int isFloatLit(const char *val);

int isValidVariable(const char *val);

int isIntLit(const char *val);

int isValidStringLit(const char *val);

ASTNode createValNode(Token current_token, NodeTypes fatherType);

const OperatorInfo *getOperatorInfo(TokenType type);

int isTypeToken(TokenType type);

NodeTypes getReturnTypeFromToken(TokenType type);

ASTNode parseFunction(Token *current);

ASTNode parseFunctionCall(Token *current, char *functionName);

// Public function prototypes
ASTNode ASTGenerator(Token token);

void printAST(ASTNode node, int depth);

void freeAST(ASTNode node);

#endif //PARSER_H
