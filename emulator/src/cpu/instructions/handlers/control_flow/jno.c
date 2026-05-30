#include "../../inst.h"

void jno_handler(CPU *cpu, Instruction *inst) {
    if (cpu->FR.O) return;
    const u16 address = operand_read(cpu, inst->ops[0]);

    cpu->PC = address;
}