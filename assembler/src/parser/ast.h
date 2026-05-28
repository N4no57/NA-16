#ifndef NA_16_AST_H
#define NA_16_AST_H

typedef struct Node_Operand {
    struct {
        registers_t reg;
        i64 immediate;

        char *symbol_name;
    };
    operand_types kind;
    Position pos;
} NodeOperand;

typedef struct Node_Instruction {
    char *mnemonic;
    NodeOperand operands[3];
    u8 operand_count;
    u8 operand_size;
    Position pos;
} NodeInstruction;

typedef enum StatementKind {
    ST_NONE,
    ST_INSTRUCTION,
    ST_SYMBOL
} StatementKind;

typedef struct Node_Symbol {
    char *symbol_name;
    i32 value;
    Position pos;
} NodeSymbol;

typedef struct Node_Statement {
    union {
        NodeInstruction instruction;
        NodeSymbol symbol;
    };
    StatementKind kind;
} NodeStatement;

typedef struct Node_Program {
    NodeStatement *statements;
    u64 count;
    u64 size;
} NodeProgram;

i32 match_signature(const NodeInstruction* inst, const InstructionSignature* sig);

#endif //NA_16_AST_H
