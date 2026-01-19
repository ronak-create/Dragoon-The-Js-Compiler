#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "parser.h"

typedef enum {
    TYPE_NUMBER,
    TYPE_STRING,
    TYPE_BOOLEAN,
    TYPE_UNKNOWN
} SemType;

SemType semantic_get_type(const char *name);


// Entry point for semantic analysis
void semantic_analyze(ASTNode *root);

#endif
