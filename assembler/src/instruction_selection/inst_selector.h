#ifndef NA_16_INST_SELECTOR_H
#define NA_16_INST_SELECTOR_H

#include "../parser/ast.h"

typedef enum {
    MOV,
    PUSH,
    POP,
    PSO_NONE
} pseudo_op;

void lowerer(NodeProgram *ast);

#endif //NA_16_INST_SELECTOR_H
