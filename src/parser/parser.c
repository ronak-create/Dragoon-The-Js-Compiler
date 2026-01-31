#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/parser.h"
#include "../../include/lexer.h"

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,
    PREC_EQUALITY,
    PREC_COMPARISON,
    PREC_TERM,
    PREC_FACTOR,
    PREC_UNARY,
    PREC_PRIMARY
} Precedence;

ASTNode *parse_expression(Token tokens[], int *index);

static Precedence get_precedence(Token *token) {
    if (token->type != TOKEN_OPERATOR) return PREC_NONE;

    if (strcmp(token->lexeme, "===") == 0 ||
        strcmp(token->lexeme, "!==") == 0)
        return PREC_EQUALITY;

    if (strcmp(token->lexeme, "<") == 0 ||
        strcmp(token->lexeme, ">") == 0 ||
        strcmp(token->lexeme, "<=") == 0 ||
        strcmp(token->lexeme, ">=") == 0)
        return PREC_COMPARISON;

    if (strcmp(token->lexeme, "+") == 0 ||
        strcmp(token->lexeme, "-") == 0)
        return PREC_TERM;

    if (strcmp(token->lexeme, "*") == 0 ||
        strcmp(token->lexeme, "/") == 0)
        return PREC_FACTOR;

    return PREC_NONE;
}

static ASTNode *parse_primary(Token tokens[], int *index) {
    Token t = tokens[*index];

    if (t.type == TOKEN_NUMBER || t.type == TOKEN_STRING ||
        t.type == TOKEN_BOOLEAN) {
        (*index)++;
        return create_node(AST_LITERAL, t.lexeme);
    }

    if (t.type == TOKEN_IDENTIFIER) {
        (*index)++;
        return create_node(AST_IDENTIFIER, t.lexeme);
    }

    if (strcmp(t.lexeme, "(") == 0) {
        (*index)++;
        ASTNode *expr = parse_expression(tokens, index);
        if (strcmp(tokens[*index].lexeme, ")") != 0) {
            printf("Expected ')'\n");
            exit(1);
        }
        (*index)++;
        return expr;
    }

    printf("Unexpected token: %s\n", t.lexeme);
    exit(1);
}

ASTNode *parse_expression_prec(Token tokens[], int *index, Precedence prec) {
    ASTNode *left = parse_primary(tokens, index);

    while (1) {
        Precedence next_prec = get_precedence(&tokens[*index]);
        if (next_prec < prec)
            break;

        Token op = tokens[*index];
        (*index)++;

        ASTNode *right = parse_expression_prec(tokens, index, next_prec + 1);

        ASTNode *bin = create_node(AST_BINARY_OP, op.lexeme);
        bin->left = left;
        bin->right = right;
        left = bin;
    }

    return left;
}


static char *strdup_safe(const char *s)
{
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);
    if (!copy) {
        perror("malloc");
        exit(1);
    }
    memcpy(copy, s, len);
    return copy;
}


ASTNode *parse_statement(Token tokens[], int *index);

ASTNode *create_node(ASTNodeType type, char *value)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = type;
    node->value = value ? strdup_safe(value) : NULL;
    node->left = node->right = NULL;
    node->body = NULL;
    node->body_size = 0;
    return node;
}

char *check_binary_expr(char *leftType, char *rightType, char op)
{
    if (strcmp(leftType, "number") == 0 && strcmp(rightType, "number") == 0)
    {
        return "number";
    }
    if (op == '+' && (strcmp(leftType, "string") == 0 || strcmp(rightType, "string") == 0))
    {
        return "string";
    }
    printf("Type Error: Cannot apply '%c' to %s and %s\n", op, leftType, rightType);
    exit(1);
}

ASTNode *parse_expression(Token tokens[], int *index) {
    return parse_expression_prec(tokens, index, PREC_ASSIGNMENT);
}

ASTNode *parse_assignment(Token tokens[], int *index)
{
    ASTNode *identifier = create_node(AST_IDENTIFIER, tokens[*index].lexeme);
    (*index)++;
    (*index)++; // Skip "="
    ASTNode *assignNode = create_node(AST_ASSIGNMENT, "=");
    assignNode->left = identifier;
    assignNode->right = parse_expression(tokens, index);
    (*index)++; // Skip ";"
    return assignNode;
}

ASTNode *parse_declaration(Token tokens[], int *index)
{
    // Token keyword = tokens[*index];
    (*index)++;
    Token identifier = tokens[*index];
    (*index)++;
    (*index)++; // Skip "="
    // Token value = tokens[*index];

    ASTNode *varNode = create_node(AST_VAR_DECL, identifier.lexeme);
    ASTNode *assignNode = create_node(AST_ASSIGNMENT, "=");
    assignNode->left = varNode;
    assignNode->right = parse_expression(tokens, index);
    
    
    (*index)++; // Skip ";"
    return assignNode;
}

ASTNode *parse_print_stmt(Token tokens[], int *index)
{
    (*index)++; // Skip "console"
    (*index)++; // Skip "."
    (*index)++; // Skip "log"
    (*index)++; // Skip "("
    ASTNode *expr = parse_expression(tokens, index);
    (*index)++; // Skip ")"
    (*index)++; // Skip ";"
    
    ASTNode *funcCall = create_node(AST_FUNC_CALL, "console.log");
    funcCall->body = malloc(sizeof(ASTNode*));
    funcCall->body[0] = expr;
    funcCall->body_size = 1;
    return funcCall;
}

ASTNode *parser_conditional_statement(Token tokens[], int *index)
{
    Token conditionKey = tokens[*index];
    (*index)++; // Skip "if" or "else"
    ASTNode *condition = NULL;

    if (strcmp(conditionKey.lexeme, "if") == 0)
    {
        if (tokens[*index].type != TOKEN_PUNCTUATION || strcmp(tokens[*index].lexeme, "(") != 0)
        {
            printf("Error: Expected '(' after 'if'\n");
            exit(1);
        }

        (*index)++; // Skip "("
        condition = parse_expression(tokens, index);

        if (tokens[*index].type != TOKEN_PUNCTUATION || strcmp(tokens[*index].lexeme, ")") != 0)
        {
            printf("Error: Expected ')' after condition\n");
            exit(1);
        }
        (*index)++; // Skip ")"
    }

    if (tokens[*index].type != TOKEN_PARENTHESES || strcmp(tokens[*index].lexeme, "{") != 0)
    {
        printf("Error: Expected '{' after condition\n");
        exit(1);
    }
    (*index)++; // Skip "{"
    
    ASTNode *block = create_node(AST_BLOCK, NULL);
    block->body = malloc(sizeof(ASTNode *) * 10);
    block->body_size = 0;
    int capacity = 10;

    while (!(tokens[*index].type == TOKEN_PARENTHESES && strcmp(tokens[*index].lexeme, "}") == 0))
    {
        if (tokens[*index].type == TOKEN_EOF)
        {
            printf("Error: Unexpected end of file. Missing closing '}'.\n");
            exit(1);
        }

        if (block->body_size >= capacity)
        {
            capacity *= 2;
            block->body = realloc(block->body, sizeof(ASTNode *) * capacity);
        }

        block->body[block->body_size++] = parse_statement(tokens, index);
    }
    (*index)++; // Skip "}"

    ASTNode *conditionNode = create_node(
        strcmp(conditionKey.lexeme, "if") == 0 ? AST_IF_STMT : AST_ELSE_STMT,
        conditionKey.lexeme
    );
    
    if (strcmp(conditionKey.lexeme, "if") == 0)
    {
        conditionNode->left = condition;
    }
    conditionNode->right = block;
    return conditionNode;
}

ASTNode *parse_update(Token tokens[], int *index)
{
    if (strcmp(tokens[*index].lexeme, "+") == 0 || strcmp(tokens[*index].lexeme, "-") == 0)
    {
        // Pre-increment: ++i or --i
        ASTNode *updateNode = create_node(AST_PRE_UPDATE, NULL);
        size_t len =
    strlen(tokens[*index].lexeme) +
    strlen(tokens[*index + 1].lexeme) +
    strlen(tokens[*index + 2].lexeme) + 1;

char *buffer = malloc(len);
if (!buffer) {
    fprintf(stderr, "Out of memory\n");
    exit(1);
}

snprintf(buffer, len, "%s%s%s",
         tokens[*index].lexeme,
         tokens[*index + 1].lexeme,
         tokens[*index + 2].lexeme);

updateNode->value = buffer;

        (*index) += 3;
        return updateNode;
    }
    else if (tokens[*index].type == TOKEN_IDENTIFIER)
    {
        // Post-increment: i++ or i--
        ASTNode *updateNode = create_node(AST_POST_UPDATE, NULL);
        size_t len =
    strlen(tokens[*index].lexeme) +
    strlen(tokens[*index + 1].lexeme) +
    strlen(tokens[*index + 2].lexeme) + 1;

char *buffer = malloc(len);
if (!buffer) {
    fprintf(stderr, "Out of memory\n");
    exit(1);
}

snprintf(buffer, len, "%s%s%s",
         tokens[*index].lexeme,
         tokens[*index + 1].lexeme,
         tokens[*index + 2].lexeme);

updateNode->value = buffer;

        (*index) += 3;
        return updateNode;
    }
    return NULL;
}

// New function to handle for loop initialization
ASTNode *parse_for_init(Token tokens[], int *index)
{
    // Check if it's a declaration (let/const) or just an assignment
    if (tokens[*index].type == TOKEN_KEYWORD && 
        (strcmp(tokens[*index].lexeme, "let") == 0 || strcmp(tokens[*index].lexeme, "const") == 0))
    {
        return parse_declaration(tokens, index);
    }
    else if (tokens[*index].type == TOKEN_IDENTIFIER)
    {
        // Simple assignment like: i = 0
        Token identifier = tokens[*index];
        (*index)++;
        
        if (tokens[*index].type != TOKEN_OPERATOR || strcmp(tokens[*index].lexeme, "=") != 0)
        {
            printf("Error: Expected '=' in for loop initialization\n");
            exit(1);
        }
        (*index)++; // Skip "="
        
        ASTNode *assignNode = create_node(AST_ASSIGNMENT, "=");
        assignNode->left = create_node(AST_IDENTIFIER, identifier.lexeme);
        assignNode->right = parse_expression(tokens, index);
        
        // Note: We don't insert into symbol table for undeclared variables in for loops
        // This is technically a semantic error, but we'll parse it
        
        (*index)++; // Skip ";"
        return assignNode;
    }
    
    printf("Error: Invalid for loop initialization\n");
    exit(1);
}

ASTNode *parser_looping_statement(Token tokens[], int *index)
{
    Token loopKey = tokens[*index];
    (*index)++; // Skip "for" or "while"
    
    if (strcmp(tokens[*index].lexeme, "(") != 0)
    {
        printf("Error: Expected '('\n");
        return NULL;
    }
    (*index)++; // Skip "("
    
    if (strcmp(loopKey.lexeme, "while") == 0)
    {
        ASTNode *condition = parse_expression(tokens, index);
        if (strcmp(tokens[*index].lexeme, ")") != 0)
        {
            printf("Error: Expected ')'\n");
            return NULL;
        }
        (*index)++; // Skip ")"
        
        if (tokens[*index].type != TOKEN_PARENTHESES || strcmp(tokens[*index].lexeme, "{") != 0)
        {
            printf("Error: Expected '{' after while condition\n");
            exit(1);
        }
        (*index)++; // Skip "{"
        
        ASTNode *block = create_node(AST_BLOCK, NULL);
        block->body = malloc(sizeof(ASTNode *) * 10);
        block->body_size = 0;
        int capacity = 10;

        while (!(tokens[*index].type == TOKEN_PARENTHESES && strcmp(tokens[*index].lexeme, "}") == 0))
        {
            if (tokens[*index].type == TOKEN_EOF)
            {
                printf("Error: Unexpected end of file. Missing closing '}'.\n");
                exit(1);
            }

            if (block->body_size >= capacity)
            {
                capacity *= 2;
                block->body = realloc(block->body, sizeof(ASTNode *) * capacity);
            }

            block->body[block->body_size++] = parse_statement(tokens, index);
        }
        (*index)++; // Skip "}"

        ASTNode *whileNode = create_node(AST_WHILE_STMT, "while");
        whileNode->left = condition;
        whileNode->right = block;
        return whileNode;
    }
    else if (strcmp(loopKey.lexeme, "for") == 0)
    {
        // Parse: for (init; condition; update)
        ASTNode *init = parse_for_init(tokens, index);  // Now handles both declarations and assignments
        ASTNode *condition = parse_expression(tokens, index);
        (*index)++; // Skip ";"
        ASTNode *update = parse_update(tokens, index);
        
        if (strcmp(tokens[*index].lexeme, ")") != 0)
        {
            printf("Error: Expected ')' after for loop header\n");
            exit(1);
        }
        (*index)++; // Skip ")"
        
        if (tokens[*index].type != TOKEN_PARENTHESES || strcmp(tokens[*index].lexeme, "{") != 0)
        {
            printf("Error: Expected '{' after for loop header\n");
            exit(1);
        }
        (*index)++; // Skip "{"
        
        ASTNode *block = create_node(AST_BLOCK, NULL);
        block->body = malloc(sizeof(ASTNode *) * 10);
        block->body_size = 0;
        int capacity = 10;

        while (!(tokens[*index].type == TOKEN_PARENTHESES && strcmp(tokens[*index].lexeme, "}") == 0))
        {
            if (tokens[*index].type == TOKEN_EOF)
            {
                printf("Error: Unexpected end of file. Missing closing '}'.\n");
                exit(1);
            }

            if (block->body_size >= capacity)
            {
                capacity *= 2;
                block->body = realloc(block->body, sizeof(ASTNode *) * capacity);
            }

            block->body[block->body_size++] = parse_statement(tokens, index);
        }
        (*index)++; // Skip "}"

        // Build proper for loop structure
        ASTNode *forNode = create_node(AST_FOR_STMT, "for");
        
        // Create a proper structure:
        // forNode->left = init
        // forNode->right = a helper node that contains condition, update, and body
        ASTNode *loopParts = create_node(AST_BLOCK, NULL);
        loopParts->body = malloc(sizeof(ASTNode*) * 3);
        loopParts->body[0] = condition;
        loopParts->body[1] = update;
        loopParts->body[2] = block;
        loopParts->body_size = 3;
        
        forNode->left = init;
        forNode->right = loopParts;
        
        return forNode;
    }
    return NULL;
}

ASTNode *parse_statement(Token tokens[], int *index)
{
    if (strcmp(tokens[*index].lexeme, "console") == 0)
    {
        return parse_print_stmt(tokens, index);
    }
    
    if (tokens[*index].type == TOKEN_KEYWORD)
    {
        if (strcmp(tokens[*index].lexeme, "let") == 0 || strcmp(tokens[*index].lexeme, "const") == 0)
        {
            return parse_declaration(tokens, index);
        }
        else if (strcmp(tokens[*index].lexeme, "if") == 0 || strcmp(tokens[*index].lexeme, "else") == 0)
        {
            return parser_conditional_statement(tokens, index);
        }
        else if (strcmp(tokens[*index].lexeme, "for") == 0 || strcmp(tokens[*index].lexeme, "while") == 0)
        {
            return parser_looping_statement(tokens, index);
        }
    }
    
    if (tokens[*index].type == TOKEN_IDENTIFIER && 
        tokens[(*index) + 1].type == TOKEN_OPERATOR && 
        strcmp(tokens[(*index) + 1].lexeme, "=") == 0)
    {
        return parse_assignment(tokens, index);
    }
    
    if (tokens[*index].type == TOKEN_EOF)
    {
        return NULL;
    }
    
    printf("Unexpected token: %s, with line: %d, skipping...\n", tokens[*index].lexeme, tokens[*index].line);
    (*index)++;
    return NULL;
}


ASTNode *fold_constants(ASTNode *node)
{
    if (!node)
        return NULL;
    node->left = fold_constants(node->left);
    node->right = fold_constants(node->right);

    if (node->type == AST_BINARY_OP && node->left && node->right)
    {
        if (node->left->type == AST_LITERAL && node->right->type == AST_LITERAL)
        {
            int left_val = atoi(node->left->value);
            int right_val = atoi(node->right->value);
            int result;
            if (strcmp(node->value, "+") == 0)
                result = left_val + right_val;
            else if (strcmp(node->value, "-") == 0)
                result = left_val - right_val;
            else if (strcmp(node->value, "*") == 0)
                result = left_val * right_val;
            else if (strcmp(node->value, "/") == 0)
                result = right_val != 0 ? left_val / right_val : 0;

            char buffer[20];
            sprintf(buffer, "%d", result);
            return create_node(AST_LITERAL, buffer);
        }
    }
    return node;
}

ASTNode *eliminate_dead_code(ASTNode *node)
{
    if (!node)
        return NULL;
    if (node->type == AST_IF_STMT)
    {
        node->left = fold_constants(node->left);
        if (node->left->type == AST_LITERAL)
        {
            if (atoi(node->left->value) == 0)
                return NULL;
            return eliminate_dead_code(node->right);
        }
    }
    return node;
}

void print_ast(ASTNode *node, int depth)
{
    if (!node) return;
    
    for (int i = 0; i < depth; i++)
        printf("  ");

    if (node->type == AST_VAR_DECL)
        printf("VarDecl(%s)\n", node->value);
    else if (node->type == AST_ASSIGNMENT)
    {
        printf("Assign\n");
        print_ast(node->left, depth + 1);
        print_ast(node->right, depth + 1);
    }
    else if (node->type == AST_BINARY_OP)
    {
        printf("BinaryOp(%s)\n", node->value);
        print_ast(node->left, depth + 1);
        print_ast(node->right, depth + 1);
    }
    else if (node->type == AST_IDENTIFIER)
        printf("Identifier(%s)\n", node->value);
    else if (node->type == AST_FUNC_CALL)
    {
        printf("FuncCall(%s)\n", node->value);
        for (int i = 0; i < node->body_size; i++)
        {
            print_ast(node->body[i], depth + 1);
        }
    }
    else if (node->type == AST_LITERAL)
        printf("Literal(%s)\n", node->value);
    else if (node->type == AST_IF_STMT)
    {
        printf("IfStmt\n");
        print_ast(node->left, depth + 1);
        print_ast(node->right, depth + 1);
    }
    else if (node->type == AST_WHILE_STMT)
    {
        printf("WhileStmt\n");
        print_ast(node->left, depth + 1);
        print_ast(node->right, depth + 1);
    }
    else if (node->type == AST_POST_UPDATE)
    {
        printf("PostUpdate(%s)\n", node->value);
    }
    else if (node->type == AST_PRE_UPDATE)
    {
        printf("PreUpdate(%s)\n", node->value);
    }
    else if (node->type == AST_FOR_STMT)
    {
        printf("ForStmt\n");
        // Print init
        for (int i = 0; i < depth + 1; i++) printf("  ");
        printf("Init:\n");
        print_ast(node->left, depth + 2);
        
        // Print condition, update, and body
        if (node->right && node->right->body_size >= 3)
        {
            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("Condition:\n");
            print_ast(node->right->body[0], depth + 2);
            
            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("Update:\n");
            print_ast(node->right->body[1], depth + 2);
            
            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("Body:\n");
            print_ast(node->right->body[2], depth + 2);
        }
    }
    else if (node->type == AST_ELSE_STMT)
    {
        printf("ElseStmt\n");
        print_ast(node->right, depth + 1);
    }
    else if (node->type == AST_BLOCK)
    {
        printf("Block\n");
        for (int i = 0; i < node->body_size; i++)
        {
            print_ast(node->body[i], depth + 1);
        }
    }
}

void free_ast(ASTNode *node)
{
    if (node == NULL)
        return;

    free_ast(node->left);
    free_ast(node->right);
    for (int i = 0; i < node->body_size; i++)
    {
        free_ast(node->body[i]);
    }
    free(node->body);
    free(node->value);
    free(node);
}