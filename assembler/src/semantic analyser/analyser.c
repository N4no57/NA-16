#include <string.h>

#include "../lib/error.h"
#include "analyser.h"

i32 validate_instruction(const NodeInstruction* inst) {
    InstructionSpec spec = get_spec((char *)inst->mnemonic);

    if (spec.mnemonic == nullptr) return 0;

    for (i32 j = 0; j < spec.signature_count; j++) {
        if (match_signature(inst, &spec.signatures[j])) return 1;
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