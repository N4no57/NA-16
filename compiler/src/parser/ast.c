#include "ast.h"

#include <stdlib.h>

void translation_unit_init(TranslationUnit *unit) {
    unit->function_count = 8;
    unit->function_capacity = 8;
    unit->functions = malloc(sizeof(TranslationUnit) * unit->function_capacity);
}

void push_function(TranslationUnit *unit, const FunctionDefinition *function) {
    if (unit->function_count == unit->function_capacity) {
        unit->function_capacity *= 2;
        FunctionDefinition *tmp = realloc(unit->functions, sizeof(FunctionDefinition) * unit->function_capacity);
        if (!tmp) {

        }
        unit->functions = tmp;
    }

    unit->functions[unit->function_count] = *function;
}

void translation_unit_destroy(const TranslationUnit *unit) {
    free(unit->functions);
}
