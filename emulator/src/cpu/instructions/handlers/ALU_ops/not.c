#include "../../inst.h"

void not_handler(CPU *cpu, Instruction *inst) {
    const u16 l = operand_read(cpu, inst->ops[1]);

    operand_write(cpu, inst->ops[0], ~l);
}