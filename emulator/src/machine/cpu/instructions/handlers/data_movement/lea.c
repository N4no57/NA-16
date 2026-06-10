#include "../../inst.h"

void lea_handler(Machine *machine, Instruction *inst) {
    // most complex one

    if (inst->ops[1].mode == 2) { // Reg indirect
        operand_write(machine, inst->ops[0], read_reg(machine, inst->ops[1].reg));
    }
}