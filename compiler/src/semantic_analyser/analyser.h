#ifndef NA_16_ANALYSER_H
#define NA_16_ANALYSER_H

#include "../target/target.h"
#include "../parser/ast.h"
#include "../type.h"

typedef struct SemanticContext {
    const TargetInfo *target;
    const CType *current_function_return_type;
} SemanticContext;

bool analyse_function_definition(SemanticContext *context, const FunctionDefinition *function);

#endif //NA_16_ANALYSER_H
