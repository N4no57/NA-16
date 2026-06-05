#include <string.h>

#include "../lib/error.h"
#include "analyser.h"

i32 validate_registers(const NodeInstruction* inst, const InstructionSpec* spec) {
    char buff[MAXTEMPSIZE];
    strcpy(buff, spec->mnemonic);
    toUpper((u8 *)buff);

    if (strcmp(buff, "MOV") == 0) {
        return 1;
    }

    if (strcmp(buff, "PUSH") == 0) {
        return 1;
    }

    if (strcmp(buff, "POP") == 0) {
        return 1;
    }

    for (u64 i = 0; i < inst->operand_count; i++) {
        if (inst->operands[i].reg >= PC) {
            error(inst->operands[0].pos, "Invalid register (Must be a GP Register)");
            return 0;
        }
    }

    return 1;
}

i32 validate_instruction(const NodeInstruction* inst) {
    InstructionSpec spec = get_spec((char *)inst->mnemonic);

    if (spec.mnemonic == nullptr) return 0;

    // handle special case
    if (is_cond_jump(spec.mnemonic)) {
        return operand_matches_class(inst->operands[0].kind, CLASS_DISP_OR_SYM, true);
    }

    for (i32 i = 0; i < spec.operand_pattern.operand_count; i++) {
        OperandClass cls = i == 0 ? CLASS_DEST : CLASS_SOURCE;
        if (spec.class == 2 && spec.opcode == 0) cls = CLASS_SOURCE; // jmp
        if (spec.class == 2 && spec.opcode == 0x10) cls = CLASS_SOURCE; // jmp

        if (operand_matches_class(inst->operands[i].kind, cls, false)) {
            return validate_registers(inst, &spec);
        }
    }

    if (spec.class == 3) {
        // system instructions
        return 1; // most have 0 operands and will fall here
    }

    if (spec.class == 2 && spec.opcode == 0xF) return 1; // ret

    return 0;
}

void analyse(const NodeProgram* ast) {
    for (u64 i = 0; i < ast->count; i++) {
        if (ast->statements[i].kind == ST_INSTRUCTION) {
            i32 old_error_count = error_count;
            i32 status = validate_instruction(&ast->statements[i].instruction);
            if (status == 0 && old_error_count == error_count) {
                error(ast->statements[i].instruction.pos,
                    "Invalid instruction \"%s\"",
                    ast->statements[i].instruction.mnemonic
                );
            }
        }
    }
}