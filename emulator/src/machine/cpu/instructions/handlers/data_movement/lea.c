#include "../../inst.h"

bool lea_handler(Machine *machine, Instruction *inst) {
    // most complex one

    bool success;
    u64 address;
    if (inst->ops[1].mode == OP_REG_IND_DISP) { // Reg indirect
        address = read_reg(machine, inst->ops[1].reg);
        address += inst->ops[1].displacement;

        success = operand_write(machine, inst->ops[0], address);
        return success;
    }

    if (inst->ops[1].mode == OP_SIB) {
        address = read_reg(machine, inst->ops[1].reg);
        address += read_reg(machine, inst->ops[1].idx_reg) << inst->ops[1].scale;

        success = operand_write(machine, inst->ops[0], address);
        return success;
    }

    if (inst->ops[1].mode == OP_SIB_DISP) {
        address = read_reg(machine, inst->ops[1].reg);
        address += read_reg(machine, inst->ops[1].idx_reg) << inst->ops[1].scale;
        address += inst->ops[1].displacement;

        success = operand_write(machine, inst->ops[0], address);
        return success;
    }

    return false;
}