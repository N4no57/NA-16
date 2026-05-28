#include <string.h>

#include "../lib/error.h"
#include "analyser.h"

i32 validate_registers(const NodeInstruction* inst, const InstructionSpec* spec) {
    char buff[MAXTEMPSIZE];
    strcpy(buff, spec->mnemonic);
    toUpper((u8 *)buff);

    if (strcmp(buff, "MOVSR") == 0) {
        if (inst->operands[0].reg >= PC) {
            error(inst->operands[0].pos, "Invalid register (Must be a GP Register)");
            return 0;
        }

        if (inst->operands[1].reg < PC) {
            error(inst->operands[1].pos, "Invalid register (Must be a SP Register)");
            return 0;
        }

        return 1;
    }

    if (strcmp(buff, "MOVRS") == 0) {
        if (inst->operands[0].reg < PC) {
            error(inst->operands[0].pos, "Invalid register (Must be a SP Register)");
            return 0;
        }

        if (inst->operands[1].reg >= PC) {
            error(inst->operands[1].pos, "Invalid register (Must be a GP Register)");
            return 0;
        }

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

    for (i32 j = 0; j < spec.signature_count; j++) {
        if (match_signature(inst, &spec.signatures[j])) {
            return validate_registers(inst, &spec);
        }
    }

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