#ifndef QBE_CODEGEN_H
#define QBE_CODEGEN_H

#include "ir.h"

void qbe_codegen_ir(IRInstr *ir, int ir_count, const char *out_qbe);

#endif
