#include "../../inst.h"

void jns_handler(CPU *cpu, Instruction *inst) {
    if (cpu->FR.N) return;
    const u16 address = operand_read(cpu, inst->ops[0]);

    cpu->PC = address;
}