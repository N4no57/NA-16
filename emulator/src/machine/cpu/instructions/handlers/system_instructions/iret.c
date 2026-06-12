#include "../../inst.h"
#include "../../../../machine.h"

bool iret_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    u64 temp;

    // old Program Counter
    if (!pop_word(machine, &temp)) return false;
    machine->cpu.sys.PC = temp;

    // flags
    if (!pop_word(machine, &temp)) return false;
    flags old_flags = cpu->sys.FR;
    cpu->sys.FR.flags = temp;

    // detect transition
    if (old_flags.U != cpu->sys.FR.U) {
        const u16 tmp = cpu->sys.SP;
        cpu->sys.SP = cpu->sys.KSP;
        cpu->sys.KSP = tmp;
    }

    return true;
}