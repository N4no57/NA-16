#include "../../inst.h"

void movs_handler(Machine *machine, Instruction *inst) {
    inst->ops[1].reg += 0x40;
    const u16 source = operand_read(machine, inst->ops[1]);

    inst->ops[0].reg += 0x40;
    operand_write(machine, inst->ops[0], source);
}