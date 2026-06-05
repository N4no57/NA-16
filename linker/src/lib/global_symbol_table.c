#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "global_symbol_table.h"
#include "../../../assembler/src/lib/error.h"

GlobalSymbolTable *glt_init(GlobalSymbolTable *table) {
    table->count = 0;
    table->size = 8;
    table->items = malloc(table->size * sizeof(table->items[0]));

    table->global_symbols.count = 0;
    table->global_symbols.size = 8;
    table->global_symbols.symbols = malloc(table->global_symbols.size * sizeof(Symbol));
    table->global_symbols.file_refs = malloc(table->global_symbols.size * sizeof(u64));

    return table;
}

void glt_free(const GlobalSymbolTable *table) {
    free(table->global_symbols.symbols);
    free(table->global_symbols.file_refs);

    free(table->items);
}

void glt_push_globsym(GlobalSymbolTable *table, const Symbol *symbol, u64 file_ref) {
    if (table->global_symbols.count >= table->global_symbols.size) {
        table->global_symbols.size *= 2;
        Symbol *tmp = realloc(table->global_symbols.symbols, table->global_symbols.size * sizeof(Symbol));
        u64 *tmp2 = realloc(table->global_symbols.file_refs, table->global_symbols.size * sizeof(u64));
        if (!tmp) {
            exit(-1);
        }
        if (!tmp2) {
            exit(-1);
        }
        table->global_symbols.symbols = tmp;
        table->global_symbols.file_refs = tmp2;
    }

    table->global_symbols.symbols[table->global_symbols.count] = *symbol;
    table->global_symbols.file_refs[table->global_symbols.count] = file_ref;
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
            Symbol *sym = glt_get_global(table, symbol->name);
            if (sym) {
                u64 tmp1 = (u64)sym;
                u64 tmp2 = (u64)table->global_symbols.symbols;
                u64 idx = (tmp1 - tmp2) / sizeof(Symbol);
                char *og_file = table->items[table->global_symbols.file_refs[idx]].filename;
                printf("\"%s\" is already declared as a global in %s but is redifined as global elsewhere\n", sym->name, og_file);
                exit(-1);
            }
            glt_push_globsym(table, symbol, table->count);
        }
    }

    table->count++;
}

Symbol *glt_get_global(GlobalSymbolTable *table, const char *symbol_name) {
    for (u64 i = 0; i < table->global_symbols.count; i++) {
        if (strcmp(table->global_symbols.symbols[i].name, symbol_name) == 0) {
            return &table->global_symbols.symbols[i];
        }
    }

    return nullptr;
}