#ifndef OPT_H
#define OPT_H

#include "parser.h"
#include "cfg.h"

void opt_constant_folding(void);
void opt_dead_code_elimination(void);

ASTNode *opt_fold_constants(ASTNode *node);

#endif
