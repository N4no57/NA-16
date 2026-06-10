#include "../../inst.h"

void jle_handler(CPU *cpu, Instruction *inst) {
    if (!cpu->FR.Z && cpu->FR.N == cpu->FR.O) return;

    cpu->PC += inst->ops[0].displacement;
}