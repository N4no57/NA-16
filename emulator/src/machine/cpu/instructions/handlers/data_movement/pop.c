#include "../../inst.h"
#include "../../../../ram/memory.h"

bool pop_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    u64 source = 0;
    bool success;

    if (inst->ops[0].size == 2) {
        success = pop_word(machine, &source);
        if (!success) return false;
    } else if (inst->ops[0].size == 1) {
        success = read_byte(machine, ++cpu->sys.SP, &source);
        if (!success) return false;
    }

    success = operand_write(machine, inst->ops[0], source);
    return success;
}