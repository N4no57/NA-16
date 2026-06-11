#include "../../inst.h"

bool jmp_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    u64 address;
    const bool success = operand_read(machine, inst->ops[0], &address);
    if (!success) return false;

    cpu->sys.PC = address;
    return true;
}