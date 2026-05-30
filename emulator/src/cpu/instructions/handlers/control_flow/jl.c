#include "../../inst.h"

void jl_handler(CPU *cpu, Instruction *inst) {
    if (cpu->FR.N == cpu->FR.O) return;
    const u16 address = operand_read(cpu, inst->ops[0]);

    cpu->PC = address;
}