#include "../../inst.h"

bool movsr_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    u64 source;

    inst->ops[1].reg += 0x40;
    if (is_privileged_reg(inst->ops[1].reg) && cpu->sys.FR.U) return false;

    bool success = operand_read(machine, inst->ops[1], &source);
    if (!success) return false;

    success = operand_write(machine, inst->ops[0], source);
    return success;
}