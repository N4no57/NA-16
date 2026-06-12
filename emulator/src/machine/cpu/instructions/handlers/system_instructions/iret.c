#include "../../inst.h"

bool iret_handler(Machine *machine, Instruction *inst) {
    u64 temp;

    // flags
    if (!pop_word(machine, &temp)) return false;
    machine->cpu.sys.FR.flags = temp;

    // old Program Counter
    if (!pop_word(machine, &temp)) return false;
    machine->cpu.sys.PC = temp;

    return true;
}