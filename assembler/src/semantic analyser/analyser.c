#include <string.h>

#include "../lib/error.h"
#include "analyser.h"

i32 validate_instruction(const NodeInstruction* inst) {
    InstructionSpec spec = get_spec((char *)inst->mnemonic);

    if (spec.mnemonic == nullptr) return 0;

    for (i32 j = 0; j < spec.signature_count; j++) {
        if (match_signature(inst, &spec.signatures[j])) return 1;
    }

    error(inst->pos, "Invalid instruction \"%s\"", inst->mnemonic);
    return 0;
}

void analyse(const NodeProgram* ast) {
    for (u64 i = 0; i < ast->count; i++) {
        if (ast->statements[i].kind == ST_INSTRUCTION) {
            validate_instruction(&ast->statements[i].instruction);
        }
    }
}