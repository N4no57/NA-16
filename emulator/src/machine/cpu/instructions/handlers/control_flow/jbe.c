#include "../../inst.h"

void jbe_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    if (!cpu->sys.FR.Z && !cpu->sys.FR.C) return;

    cpu->sys.PC += inst->ops[0].displacement;
}