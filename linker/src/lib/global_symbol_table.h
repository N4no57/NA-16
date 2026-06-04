#ifndef NA_16_GLOBAL_SYMBOL_TABLE_H
#define NA_16_GLOBAL_SYMBOL_TABLE_H

#include "../object_file_reader/obj_file.h"

typedef struct {
    struct {
        char *filename;
        Symbol *symbols;
        u64 count;
    } *items;

    struct {
        Symbol *symbols;
        u64 count;
        u64 size;
    } global_symbols;

    u64 count;
    u64 size;
} GlobalSymbolTable;

GlobalSymbolTable *glt_init(GlobalSymbolTable *table);

void glt_free(const GlobalSymbolTable *table);

void glt_push_table(GlobalSymbolTable *table, Symbol *symbols, u64 count, char *filename);

#endif //NA_16_GLOBAL_SYMBOL_TABLE_H
