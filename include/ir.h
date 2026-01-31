#ifndef IR_H
#define IR_H
#include "parser.h"

typedef enum {
    IR_ASSIGN,
    IR_BINOP,
    IR_LABEL,
    IR_GOTO,
    IR_IF_FALSE,
    IR_PARAM,
    IR_CALL
} IROp;

typedef struct {
    IROp op;
    char *dst;
    char *lhs;
    char *op_str;
    char *rhs;
    char *label;
    char *func;
    int argc;
} IRInstr;

void ir_generate(ASTNode *root);
IRInstr *ir_get_all(int *count);

#endif
