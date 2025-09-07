#include <stdio.h>
#include <string.h>

#include "lexer/lexer.h"
#include "parser/parser.h"

// Updated to work with the new Input struct.
void printTokens(Input in) {
    if (in == NULL) {
        printf("No tokens found.\n");
        return;
    }

    printf("Raw tokens from splitter:\n");
    for (int i = 0; i < in->n; i++) {
        InputToken *raw_token = &in->tokens[i];
        printf("  Split %d (L%d, C%d): '%s'\n", i, raw_token->line, raw_token->column, raw_token->value);
    }
    printf("Total raw tokens: %d\n", in->n);
}

void printTokenList(Token t) {
    if (t == NULL || t->next == NULL) {
        printf("No tokens found.\n");
        return;
    }

    int i = 0;
    // Start with t->next to skip the dummy head node.
    Token current = t->next; 
    while (current != NULL) {
        printf("Token %d (L%d, C%d): '%s', type: %d\n", 
               i, current->line, current->column, current->value, current->type);
        i++;
        current = current->next;
    }
    printf("Total tokens processed: %d\n", i);
}

int main() {
    printf("=== LEXER TEST ===\n");
    char *input = "a ? b : c ? d : e ? f : g ? h : i;";
    printf("Input:\n%s\n\n", input);

    // Use the new two-step lexer process
    printf("1. SPLITTING:\n");
    Input res = splitter(input);
    printTokens(res); // Call the updated helper function
    printf("\n");

    printf("2. TOKENIZATION:\n");
    // 'res' is consumed and freed by the tokenization function.
    Token t = tokenization(res);
    printTokenList(t);
    printf("\n");

    printf("3. AST GENERATION:\n");
    ASTNode ast = ASTGenerator(t);
    printAST(ast, 0);
    printf("\n");
    
    // Free the final data structures.
    freeTokenList(t);
    freeAST(ast);

    // printErrorSummary(); // You will need to implement this
    // int hasErrors = 0; // You will need to implement this

    printf("\nTip: Use './compiler --test' to run tests\n");

    // return hasErrors ? 1 : 0;
    return 0;
}
