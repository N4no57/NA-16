#include "../../inst.h"

void not_handler(Machine *machine, Instruction *inst) {
    const u16 l = operand_read(machine, inst->ops[1]);

    const u16 result = ~l;

    operand_write(machine, inst->ops[0], result);
    set_flags(machine, result, nullptr, 0b0011, inst->ops[0].size);
}