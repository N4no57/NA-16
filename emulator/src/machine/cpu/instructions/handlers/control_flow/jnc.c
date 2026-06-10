#include "../../inst.h"

void jnc_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    if (cpu->sys.FR.C) return;

    cpu->sys.PC += inst->ops[0].displacement;
}