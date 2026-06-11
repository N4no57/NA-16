#include "../../inst.h"

bool not_handler(Machine *machine, Instruction *inst) {
    u64 l;
    bool success = operand_read(machine, inst->ops[1], &l);
    if (!success) return false;

    const u64 result = ~l;

    success = operand_write(machine, inst->ops[0], result);
    if (!success) return false;

    set_flags(machine, result, nullptr, 0b0011, inst->ops[0].size);
    return true;
}