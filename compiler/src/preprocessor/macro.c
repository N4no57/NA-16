#include <stdlib.h>
#include <string.h>

#include "macro.h"

int macro_compare(void *a, void *b) {
    const Macro *macro_a = (Macro *)a;
    const Macro *macro_b = (Macro *)b;

    return strcmp(macro_a->name, macro_b->name);
}

Macro *macro_table_find(Vector *table, char *name) {
    Macro macro = {0};
    macro.name = name;
    return vector_find(table, &macro, macro_compare);
}

Error macro_table_define(Vector *table, const Macro macro) {
    if (macro_table_find(table, macro.name) != nullptr) {
        return ERROR_ALREADY_EXISTS;
    }

    vector_push(table, &macro);

    return ERROR_OK;
}

void macro_table_undef(Vector *table, char *name) {
    Macro *macro = macro_table_find(table, name);
    const size_t idx = (size_t)(macro - (Macro *)table->data);

    vector_swap_remove(table, idx, nullptr);

    for (size_t i = 0; i < macro->parameter_count; i++) {
        free(macro->parameters[i]);
    }

    free(macro->parameters);
    free(macro->name);
    macro->name = nullptr;
    macro->replacement = nullptr;
    macro->parameters = nullptr;
}
