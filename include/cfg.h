#ifndef CFG_H
#define CFG_H

#include "parser.h"

typedef struct BasicBlock {
    int id;
    struct BasicBlock **succ;
    int succ_count;
    ASTNode **stmts;
    int stmt_count;
} BasicBlock;

void cfg_build(ASTNode *root);
void cfg_print(void);

BasicBlock *cfg_get_block(int index);
int cfg_block_count(void);

#endif
