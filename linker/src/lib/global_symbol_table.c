#include <stdlib.h>

#include "global_symbol_table.h"

#include <string.h>

GlobalSymbolTable *glt_init(GlobalSymbolTable *table) {
    table->count = 0;
    table->size = 8;
    table->items = malloc(table->size * sizeof(table->items[0]));

    table->global_symbols.count = 0;
    table->global_symbols.size = 8;
    table->global_symbols.symbols = malloc(table->global_symbols.size * sizeof(Symbol));

    return table;
}

void glt_free(const GlobalSymbolTable *table) {
    free(table->global_symbols.symbols);

    for (u64 i = 0; i < table->global_symbols.count; i++) {
        free(table->items[i].symbols);
    }

    free(table->items);
}

void glt_push_globsym(GlobalSymbolTable *table, const Symbol *symbol) {
    if (table->global_symbols.count >= table->global_symbols.size) {
        table->global_symbols.size *= 2;
        Symbol *tmp = realloc(table->global_symbols.symbols, table->global_symbols.size * sizeof(Symbol));
        if (!tmp) {
            exit(-1);
        }
        table->global_symbols.symbols = tmp;
    }

    table->global_symbols.symbols[table->global_symbols.count] = *symbol;
    table->global_symbols.count++;
}

void glt_push_table(GlobalSymbolTable *table, Symbol *symbols, const u64 count, char *filename) {
    if (table->count >= table->size) {
        GlobalSymbolTable tmp;
        table->size *= 2;
        tmp.items = realloc(table->items, table->size * sizeof(table->items[0]));
        if (!tmp.items) {
            exit(-1);
        }
        table->items = tmp.items;
    }

    table->items[table->count].filename = filename;
    table->items[table->count].symbols = symbols;
    table->items[table->count].count = count;

    for (u64 i = 0; i < table->items[table->count].count; i++) {
        Symbol *symbol = &table->items[table->count].symbols[i];
        if (symbol->flags & SYM_GLOBAL) {
            glt_push_globsym(table, symbol);
        }
    }

    table->count++;
}

Symbol *glt_get(GlobalSymbolTable *table, const char *symbol_name, const char *filename) {
    for (u64 i = 0; i < table->count; i++) {
        if (strcmp(table->items[i].filename, filename) == 0) {
            for (u64 j = 0; j < table->items[i].count; j++) {
                if (strcmp(table->items[i].symbols[j].name, symbol_name) == 0) {
                    return &table->items[i].symbols[j];
                }
            }
        }
    }

    for (u64 i = 0; i < table->global_symbols.count; i++) {
        if (strcmp(table->global_symbols.symbols[i].name, symbol_name) == 0) {
            return &table->global_symbols.symbols[i];
        }
    }

    return nullptr;
}