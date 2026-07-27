#include "ast.h"

#include <stdlib.h>

void translation_unit_init(TranslationUnit *unit) {
    unit->count = 8;
    unit->capacity = 8;
    unit->items = malloc(sizeof(ExternalDeclaration) * unit->capacity);
}

void push_external_declaration(TranslationUnit *unit, const ExternalDeclaration *external_declaration) {
    if (unit->count == unit->capacity) {
        unit->capacity *= 2;
        ExternalDeclaration *tmp = realloc(unit->items, sizeof(FunctionDefinition) * unit->capacity);
        if (!tmp) {

        }
        unit->items = tmp;
    }

    unit->items[unit->count++] = *external_declaration;
}

void compound_statement_append(CompoundStatement *compound_statement, const BlockItem *item) {
    if (compound_statement->count == compound_statement->capacity) {
        compound_statement->capacity *= 2;
        BlockItem *tmp = realloc(compound_statement->items, sizeof(BlockItem) * compound_statement->capacity);
        if (!compound_statement->items) {

        }
        compound_statement->items = tmp;
    }

    compound_statement->items[compound_statement->count++] = *item;
}

void translation_unit_destroy(const TranslationUnit *unit) {
    free(unit->items);
}
