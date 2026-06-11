#include "../../inst.h"

bool test_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    u64 l, r;
    bool success = operand_read(machine, inst->ops[0], &l);
    if (!success) return false;

    success = operand_read(machine, inst->ops[1], &r);
    if (!success) return false;

    const u16 result = l & r;

    set_flags(machine, result, nullptr, 0b0011, inst->ops[0].size);
    cpu->sys.FR.O = cpu->sys.FR.C = 0;
    return true;
}