#include "../../inst.h"

void ja_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    if (cpu->sys.FR.Z || cpu->sys.FR.C) return;

    cpu->sys.PC += inst->ops[0].displacement;
}