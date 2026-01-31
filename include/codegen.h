#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"

void codegen_c(ASTNode *root, const char *out_file);

#endif
