#include "../../inst.h"

void add_handler(CPU *cpu, Instruction *inst) {
    const u16 l = operand_read(cpu, inst->ops[1]);
    const u16 r = operand_read(cpu, inst->ops[2]);

    operand_write(cpu, inst->ops[0], l + r);
}