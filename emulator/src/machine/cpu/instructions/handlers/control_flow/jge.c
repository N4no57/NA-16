#include "../../inst.h"

void jge_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    if (cpu->sys.FR.O != cpu->sys.FR.N) return;

    cpu->sys.PC += inst->ops[0].displacement;
}