#ifndef QBE_CODEGEN_H
#define QBE_CODEGEN_H

#include "parser.h"

/* Emit QBE IR to file */
void qbe_codegen(ASTNode *program, const char *out_qbe);

#endif
