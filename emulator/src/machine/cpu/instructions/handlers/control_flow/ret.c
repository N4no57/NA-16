#include "../../inst.h"
#include "../../../../ram/memory.h"

bool ret_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    u64 address;
    const bool success = pop_word(machine, &address);
    if (!success) return false;

    cpu->sys.PC = address;
    return true;
}
