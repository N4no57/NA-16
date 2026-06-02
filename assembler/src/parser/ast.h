#ifndef NA_16_AST_H
#define NA_16_AST_H

#include "../lib/asmlib.h"
#include "../lexer/lexer.h"

typedef struct {
    struct {
        registers_t reg;
        i64 immediate;

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

typedef struct {
    char *symbol_name;
    i64 value;
    SymbolKind kind;
    Position pos;
    bool global;
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

i32 match_signature(const NodeInstruction* inst, const InstructionSignature* sig);

#endif //NA_16_AST_H
