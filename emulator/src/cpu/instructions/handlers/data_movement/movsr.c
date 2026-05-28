#include "../../inst.h"

void movsr_handler(CPU *cpu, Instruction *inst) {
    inst->ops[1].reg++;
    inst->ops[1].reg <<= 6;
    const u16 source = operand_read(cpu, inst->ops[1]);

    operand_write(cpu, inst->ops[0], source);
}