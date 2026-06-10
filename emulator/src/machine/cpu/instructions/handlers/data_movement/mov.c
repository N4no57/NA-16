#include "../../inst.h"

void mov_handler(Machine *machine, Instruction *inst) {
    const u16 source = operand_read(machine, inst->ops[1]);

    operand_write(machine, inst->ops[0], source);
}