#ifndef NA_16_CODEGEN_H
#define NA_16_CODEGEN_H

#include "../lib/asmlib.h"
#include "../parser/ast.h"

#define GEN_MEX(DEST_MODE, SRC1_MODE, SRC2_MODE) ((0x8 << 12) | (((DEST_MODE) & 0xF) << 8) | (((SRC1_MODE) & 0xF) << 4) | ((SRC2_MODE) & 0xF))
#define GEN_AEX (0x91)

typedef struct {
    NodeSymbol *symbols;
    u64 count;
    u64 size;
} SymbolTable;

void init_symbol_table(SymbolTable *table);
void push_symbol(SymbolTable *table, NodeSymbol *symbol);

void symbol_pass(NodeProgram *ast, SymbolTable *symtbl);
void generate_code(NodeProgram *ast, bytes *code);

#endif //NA_16_CODEGEN_H
