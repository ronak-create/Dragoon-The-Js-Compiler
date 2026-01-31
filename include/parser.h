#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "lexer.h"

// typedef enum {
//     TOKEN_IDENTIFIER,
//     TOKEN_KEYWORD,
//     TOKEN_NUMBER,
//     TOKEN_STRING,
//     TOKEN_OPERATOR,
//     TOKEN_BINARY_EXPRESSION,
//     TOKEN_SEMICOLON,
//     TOKEN_PARENTHESES,
//     TOKEN_PUNCTUATION,
//     TOKEN_BOOLEAN,
//     TOKEN_COMMENT,
//     TOKEN_ERROR,
//     TOKEN_EOF
// } TokenType;

// typedef struct {
//     TokenType type;
//     char lexeme[50];
//     int line;
// } Token;


typedef enum
{
    AST_VAR_DECL,
    AST_ASSIGNMENT,
    AST_LITERAL,
    AST_BINARY_OP,
    AST_IF_STMT,
    AST_ELSE_STMT,
    AST_BLOCK,
    AST_WHILE_STMT,
    AST_FOR_STMT,
    AST_PRE_UPDATE,
    AST_POST_UPDATE,
    AST_FUNCTION,
    AST_IDENTIFIER,
    AST_LOG_STMT,
    AST_FUNC_CALL
} ASTNodeType;

typedef struct ASTNode
{
    ASTNodeType type;
    char *value;           // Variable name, operator, or literal
    struct ASTNode *left;  // LHS (for assignment, binary op)
    struct ASTNode *right; // RHS (for assignment, binary op)
    struct ASTNode **body; // For block statements
    int body_size;
} ASTNode;

#define PARSER_MAX_SYMBOLS 100

typedef struct
{
    char name[50];
    char type[50];
    char scope[50];
    char value[100];
    char datatype[50];
} Symbol;

// extern Symbol symbolTable[MAX_SYMBOLS];

void free_ast(ASTNode *node);
void print_ast(ASTNode *node, int depth);
void printSymbolTable();
void generate_tac(ASTNode *node);
ASTNode *parse_statement(Token tokens[], int *index);
ASTNode *create_node(ASTNodeType type, char *value);
// Token get_next_token(FILE *file); // This function must be implemented by you in parser.c or another file

#endif // PARSER_H