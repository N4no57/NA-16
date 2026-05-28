#include "../../inst.h"

void movrs_handler(CPU *cpu, Instruction *inst) {
    const u16 source = operand_read(cpu, inst->ops[1]);

    inst->ops[0].reg <<= 6;
    operand_write(cpu, inst->ops[0], source);
}