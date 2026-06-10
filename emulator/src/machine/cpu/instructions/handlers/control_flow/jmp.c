#include "../../inst.h"

void jmp_handler(CPU *cpu, Instruction *inst) {
    const u16 address = operand_read(cpu, inst->ops[0]);

    cpu->PC = address;
}