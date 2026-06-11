#include "../../inst.h"

bool jg_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    if (!cpu->sys.FR.Z && cpu->sys.FR.O == cpu->sys.FR.N) cpu->sys.PC += inst->ops[0].displacement;
    return true;
}