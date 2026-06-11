#include "../../inst.h"

bool cmp_handler(Machine *machine, Instruction *inst) {
    u64 l, r;
    bool success = operand_read(machine, inst->ops[0], &l);
    if (!success) return false;

    success = operand_read(machine, inst->ops[1], &r);
    if (!success) return false;

    const u64 result = l - r;

    u32 values[2] = {l, r};
    set_flags(machine, result, values, 0b1111, inst->ops[0].size);
    return true;
}