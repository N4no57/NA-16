#ifndef NA_16_CODEGEN_H
#define NA_16_CODEGEN_H

#include "../lib/asmlib.h"
#include "../parser/ast.h"
#include "../obj_file_writer/sections.h"

#define GEN_MEX(DEST_MODE, SRC1_MODE, SRC2_MODE) ((0x8 << 12) | (((DEST_MODE) & 0xF) << 8) | (((SRC1_MODE) & 0xF) << 4) | ((SRC2_MODE) & 0xF))
#define GEN_AEX (0x91)
#define GEN_ESCAPE_BYTE (0xF0)

typedef struct {
    NodeSymbol *symbols;
    u64 count;
    u64 size;
} SymbolTable;

NodeSymbol *find_symbol(const SymbolTable *table, const char *symbol);

bool wont_fit_u8(u64 value);
bool wont_fit_s8(i64 value);
bool require_16_bits(const NodeInstruction *node, const InstructionSignature *sig);

void symbol_pass(NodeProgram *ast, SymbolTable *symbols, SectionTable *sections);
void generate_code(NodeProgram *ast, SymbolTable *symbols, SectionTable *sections);

#endif //NA_16_CODEGEN_H
