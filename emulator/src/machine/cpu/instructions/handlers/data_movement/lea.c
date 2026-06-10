#include "../../inst.h"

void lea_handler(CPU *cpu, Instruction *inst) {
    // most complex one

    if (inst->ops[1].mode == 2) { // Reg indirect
        operand_write(cpu, inst->ops[0], read_reg(cpu, inst->ops[1].reg));
    }
}