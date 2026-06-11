#include "../../inst.h"

bool mov_handler(Machine *machine, Instruction *inst) {
    u64 source;
    bool success = operand_read(machine, inst->ops[1], &source);
    if (!success) return false;

    success = operand_write(machine, inst->ops[0], source);
    return success;
}