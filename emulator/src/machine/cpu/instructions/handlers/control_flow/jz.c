#include "../../inst.h"

void jz_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    if (!cpu->sys.FR.Z) return;

    cpu->sys.PC += inst->ops[0].displacement;
}