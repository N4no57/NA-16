#include "inst_selector.h"
#include "../lib/asmlib.h"

#include <string.h>

char *pseudo_op_table[] = {
    [MOV] = "MOV"
};

u64 table_size = sizeof(pseudo_op_table)/sizeof(pseudo_op_table[0]);

pseudo_op get_pseudo_op(char *mnemonic) {
    char tmp[MAXTEMPSIZE];
    strcpy(tmp, mnemonic);
    toUpper((u8 *)tmp);
    for (u64 i = 0; i < table_size; i++) {
        if (strcmp(tmp, pseudo_op_table[i]) == 0) {
            return i;
        }
    }
    return PSO_NONE;
}

void lower_mov(NodeInstruction *inst) {
    bool is_SPR[2] = {false, false};

    for (u8 i = 0; i < inst->operand_count; i++) {
        if (inst->operands[i].kind == REGISTER) {
            is_SPR[i] = isSPR(inst->operands[i].reg);
        }
    }

    if (!is_SPR[0] && !is_SPR[1]) return;

    if (!is_SPR[0] && is_SPR[1]) {
        inst->mnemonic = strdup("MOVSR");
        return;
    }

    if (is_SPR[0] && !is_SPR[1]) {
        inst->mnemonic = strdup("MOVRS");
        return;
    }

    if (is_SPR[0] && is_SPR[1]) {
        inst->mnemonic = strdup("MOVS");
    }
}

void lower_instruction(NodeInstruction *inst) {
    pseudo_op op = get_pseudo_op(inst->mnemonic);
    if (op == PSO_NONE) return;

    if (op == MOV) {
        lower_mov(inst);
        return;
    }
}

void lowerer(NodeProgram *ast) {
    for (u64 i = 0; i < ast->size; i++) {
        if (ast->statements[i].kind != ST_INSTRUCTION) continue;
        lower_instruction(&ast->statements[i].instruction);
    }
}