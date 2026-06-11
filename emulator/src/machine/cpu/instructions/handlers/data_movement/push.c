#include "../../inst.h"
#include "../../../../ram/memory.h"

bool push_handler(Machine *machine, Instruction *inst) {
    u64 source;

    bool success = operand_read(machine, inst->ops[0], &source);
    if (!success) return false;

    if (inst->ops[0].size == 2) {
        success = push_word(machine, source);
        if (!success) return false;
    } else if (inst->ops[0].size == 1) {
        success = push_byte(machine, source);
        if (!success) return false;
    }
    return true;
}