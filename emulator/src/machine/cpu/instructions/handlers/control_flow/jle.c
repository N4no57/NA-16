#include "../../inst.h"

void jle_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    if (!cpu->sys.FR.Z && cpu->sys.FR.N == cpu->sys.FR.O) return;

    cpu->sys.PC += inst->ops[0].displacement;
}