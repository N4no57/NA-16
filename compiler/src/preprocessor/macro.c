#include <stdlib.h>
#include <string.h>

#include "macro.h"

Macro *macro_table_find(const MacroTable *table, const char *name) {
    for (size_t i = 0; i < table->count; i++) {
        if (table->entries[i].name == nullptr) continue;

        if (strcmp(table->entries[i].name, name) == 0) {
            return &table->entries[i];
        }
    }

    return nullptr;
}

Error macro_table_define(MacroTable *table, const Macro macro) {
    if (macro_table_find(table, macro.name) != nullptr) {
        return ERROR_ALREADY_EXISTS;
    }

    if (table->count >= table->capacity) {
        if (table->capacity == 0) table->capacity = 8;
        else table->capacity *= 2;

        Macro *tmp = realloc(table->entries, sizeof(Macro) * table->capacity);
        if (tmp == nullptr) {
            return ERROR_ALLOCATION_FAILED;
        }

        table->entries = tmp;
    }

    table->entries[table->count] = macro;
    table->count++;

    return ERROR_OK;
}

void macro_table_undef(const MacroTable *table, const char *name) {
    Macro *macro = macro_table_find(table, name);

    for (size_t i = 0; i < macro->parameter_count; i++) {
        free(macro->parameters[i]);
    }

    free(macro->parameters);
    free(macro->name);
    macro->name = nullptr;
    macro->replacement = nullptr;
    macro->parameters = nullptr;
}
