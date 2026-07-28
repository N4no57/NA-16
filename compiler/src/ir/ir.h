#ifndef NA_16_IR_H
#define NA_16_IR_H

#include <stdint.h>

#include "../lexer/source.h"
#include "../parser/ast.h"
#include "../type.h"

typedef enum IRValueKind {
    IR_VALUE_INTEGER_CONSTANT,

    IR_VALUE_TEMPORARY
} IRValueKind;

typedef uint32_t IRValueID;

typedef struct IRValue {
    IRValueKind kind;
    const CType *type;

    union {
        uint64_t integer_constant;
        IRValueID temporary;
    } data;
} IRValue;

typedef enum IRInstructionKind {
    IR_INSTRUCTION_RETURN
} IRInstructionKind;

typedef struct IRInstruction {
    IRInstructionKind kind;
    SourceSpan source_span;

    union {
        struct {
            bool has_value;
            IRValue value;
        } return_instruction;
    } data;
} IRInstruction;

typedef uint32_t IRBlockID;

typedef struct IRBasicBlock {
    IRBlockID id;

    IRInstruction *instructions;
    size_t instruction_count;
    size_t instruction_capacity;

    bool terminated;
} IRBasicBlock;

typedef struct IRFunction {
    char *name;
    SourceSpan source_span;

    const CType *return_type;

    IRBasicBlock *blocks;
    size_t block_count;
    size_t block_capacity;

    IRBlockID entry_block;
} IRFunction;

typedef struct IRModule {
    IRFunction *functions;
    size_t function_count;
    size_t function_capacity;
} IRModule;

bool ir_instruction_append(IRBasicBlock *block, const IRInstruction *instruction);
bool ir_block_append(IRFunction *function, const IRBasicBlock *block);
bool ir_function_append(IRModule *module, const IRFunction *function);

bool lower_ast(TranslationUnit *unit, IRModule *module);

#endif //NA_16_IR_H
