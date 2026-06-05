#ifndef NA_16_AST_H
#define NA_16_AST_H

#include "../lib/asmlib.h"
#include "../lexer/lexer.h"

typedef struct {
    struct {
        registers_t reg; // when SIB this is base reg
        i64 immediate;

        // for SIB only
        registers_t idx_reg;
        u8 scale;

        // symbols duh
        char *symbol_name;
    };
    operand_types kind;
    Position pos;
} NodeOperand;

typedef struct {
    char *mnemonic;
    NodeOperand operands[3];
    u8 operand_count;
    u8 operand_size;
    Position pos;
} NodeInstruction;

typedef enum {
    ST_NONE,
    ST_INSTRUCTION,
    ST_SYMBOL,
    ST_DIRECTIVE,
} StatementKind;

typedef enum {
    SK_CONSTANT,
    SK_LABEL
} SymbolKind;

typedef enum {
    SYM_GLOBAL = 1 << 0,
    SYM_DEFINED = 1 << 1,
} SymbolFlags;

typedef struct {
    char *symbol_name;

    SymbolKind kind;
    i64 value;

    u8 flags;
    u64 section_idx;

    Position pos;
} NodeSymbol;

typedef struct {
    char *name;
    TokenList args;
    Position pos;
} NodeDirective;

typedef struct {
    union {
        NodeInstruction instruction;
        NodeSymbol symbol;
        NodeDirective directive;
    };
    StatementKind kind;
} NodeStatement;

typedef struct {
    NodeStatement *statements;
    u64 count;
    u64 size;
} NodeProgram;

#endif //NA_16_AST_H
