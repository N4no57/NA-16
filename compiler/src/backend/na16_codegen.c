#include "na16_codegen.h"

static bool emit_return(FILE *output, const IRInstruction *inst) {
    if (!inst->data.return_instruction.has_value) {
        fprintf(output, "\tret\n");
        return true;
    }

    const IRValue *value = &inst->data.return_instruction.value;

    if (value->kind != IR_VALUE_INTEGER_CONSTANT) {
        return false;
    }

    const uint64_t integer = value->data.integer_constant;

    fprintf(output, "\tmov r0, %llu\n", (uint64_t)integer);

    fprintf(output, "\tret\n");
    return true;
}

static bool emit_instruction(FILE *output, const IRInstruction *inst) {
    switch (inst->kind) {
        case IR_INSTRUCTION_RETURN:
            return emit_return(output, inst);
    }

    return false;
}

static bool emit_basic_block(FILE *output, const IRFunction *function, const IRBasicBlock *block) {
    for (size_t i = 0; i < block->instruction_count; i++) {
        if (!emit_instruction(output, &block->instructions[i])) return false;
    }

    return true;
}

static bool emit_function(FILE *output, const IRFunction *function) {
    if (output == nullptr || function == nullptr) return false;

    fprintf(output, ".global %s\n", function->name);
    fprintf(output, "%s:\n", function->name);

    for (size_t i = 0; i < function->block_count; i++) {
        if (!emit_basic_block(output, function, &function->blocks[i])) return false;
    }

    fputc('\n', output);
    return true;
}

bool na16_emit_module(FILE *output, const IRModule *module, const TargetInfo *target) {
    if (output == nullptr || module == nullptr || target == nullptr) return false;

    fprintf(output, ".section text\n");

    for (size_t i = 0; i < module->function_count; i++) {
        if (!emit_function(output, &module->functions[i])) return false;
    }

    return !ferror(output);
}
