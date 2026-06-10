#include "../../inst.h"

void movrs_handler(Machine *machine, Instruction *inst) {
    const u16 source = operand_read(machine, inst->ops[1]);

    inst->ops[0].reg += 0x40;
    operand_write(machine, inst->ops[0], source);
}