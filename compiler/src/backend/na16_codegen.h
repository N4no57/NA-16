#ifndef NA_16_NA16_CODEGEN_H
#define NA_16_NA16_CODEGEN_H

#include <stdio.h>

#include "../ir/ir.h"
#include "../target/target.h"

bool na16_emit_module(FILE *output, const IRModule *module, const TargetInfo *target);

#endif //NA_16_NA16_CODEGEN_H
