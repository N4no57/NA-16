#include "../../inst.h"

void movsr_handler(Machine *machine, Instruction *inst) {
    inst->ops[1].reg += 0x40;
    const u16 source = operand_read(machine, inst->ops[1]);

    operand_write(machine, inst->ops[0], source);
}