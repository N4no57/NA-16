#include "codegen.h"

void push_symbol(SymbolTable *table, NodeSymbol symbol) {
    if (table->count >= table->size) {
        if (table->size == 0) table->size = 256;
        else table->size *= 2;
        NodeSymbol *tmp = realloc(table->symbols, table->size * sizeof(NodeSymbol));
        if (tmp == nullptr) {
            exit(EXIT_FAILURE);
        }
        table->symbols = tmp;
    }

    table->symbols[table->count++] = symbol;
}
