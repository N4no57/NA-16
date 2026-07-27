#include "ast.h"

#include <stdlib.h>

void translation_unit_init(TranslationUnit *unit) {
    unit->count = 8;
    unit->capacity = 8;
    unit->items = malloc(sizeof(ExternalDeclaration) * unit->capacity);
}

void push_function(TranslationUnit *unit, const ExternalDeclaration *external_declaration) {
    if (unit->count == unit->capacity) {
        unit->capacity *= 2;
        ExternalDeclaration *tmp = realloc(unit->items, sizeof(FunctionDefinition) * unit->capacity);
        if (!tmp) {

        }
        unit->items = tmp;
    }

    unit->items[unit->count] = *external_declaration;
}

void translation_unit_destroy(const TranslationUnit *unit) {
    free(unit->items);
}
