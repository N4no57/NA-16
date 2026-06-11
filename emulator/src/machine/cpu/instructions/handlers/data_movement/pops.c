#include "../../inst.h"
#include "../../../../PIC/pic.h"

bool pops_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    u64 source = 0;
    bool success;

    if (inst->ops[0].size == 2) {
        success = pop_word(machine, &source);
        if (!success) return false;
    } else if (inst->ops[0].size == 1) {
        success = pop_byte(machine, &source);
        if (!success) return false;
    }

    inst->ops[0].reg += 0x40;
    if (is_privileged_reg(inst->ops[0].reg) && cpu->sys.FR.U) {
        raise_exception(machine, PV, cpu->sys.PC);
        return false;
    }

    success = operand_write(machine, inst->ops[0], source);
    return success;
}