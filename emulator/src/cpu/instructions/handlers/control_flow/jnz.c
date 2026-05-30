#include "../../inst.h"

void jnz_handler(CPU *cpu, Instruction *inst) {
    if (cpu->FR.Z) return;
    const u16 address = operand_read(cpu, inst->ops[0]);

    cpu->PC = address;
}