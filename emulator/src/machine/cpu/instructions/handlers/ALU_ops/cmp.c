#include "../../inst.h"

void cmp_handler(Machine *machine, Instruction *inst) {
    const u16 l = operand_read(machine, inst->ops[0]);
    const u16 r = operand_read(machine, inst->ops[1]);

    const u16 result = l - r;

    u32 values[2] = {l, r};
    set_flags(machine, result, values, 0b1111, inst->ops[0].size);
}