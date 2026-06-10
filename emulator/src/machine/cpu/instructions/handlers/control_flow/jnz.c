#include "../../inst.h"

void jnz_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    if (cpu->sys.FR.Z) return;

    cpu->sys.PC += inst->ops[0].displacement;
}