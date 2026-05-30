#include "../../inst.h"

void jc_handler(CPU *cpu, Instruction *inst) {
    if (!cpu->FR.C) return;
    const u16 address = operand_read(cpu, inst->ops[0]);

    cpu->PC = address;
}