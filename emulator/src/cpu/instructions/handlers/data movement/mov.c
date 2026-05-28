#include "../../inst.h"

void mov_handler(CPU *cpu, Instruction *inst) {
    const u16 source = operand_read(cpu, inst->ops[1]);

    operand_write(cpu, inst->ops[0], source);
}