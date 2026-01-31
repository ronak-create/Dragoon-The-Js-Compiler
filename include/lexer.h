#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

#define MAX_TOKEN_LENGTH 256

typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_KEYWORD,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_OPERATOR,
    TOKEN_PUNCTUATION,
    TOKEN_PARENTHESES,
    TOKEN_SEMICOLON,
    TOKEN_BOOLEAN,
    TOKEN_EOF,
    TOKEN_COMMENT,
    TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[MAX_TOKEN_LENGTH];
    int line;
} Token;

int is_builtInObject(const char *str);
int is_keyword(const char *str);
Token get_next_token(FILE *file);


#endif // LEXER_H