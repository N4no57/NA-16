#include "../../inst.h"
#include "../../../../ram/memory.h"

bool call_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    bool success = push_word(machine, cpu->sys.PC); // push return address
    if (!success) return false;

    u64 address;
    success = operand_read(machine, inst->ops[0], &address);
    if (!success) return false;

    cpu->sys.PC = address;
    return true;
}
