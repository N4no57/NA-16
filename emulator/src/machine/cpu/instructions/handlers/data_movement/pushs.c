#include "../../inst.h"
#include "../../../../ram/memory.h"

bool pushs_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    u64 source;

    inst->ops[0].reg += 0x40;
    if (is_privileged_reg(inst->ops[0].reg) && cpu->sys.FR.U) return false;

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