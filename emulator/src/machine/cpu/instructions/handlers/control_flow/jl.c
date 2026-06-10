#include "../../inst.h"

void jl_handler(CPU *cpu, Instruction *inst) {
    if (cpu->FR.N == cpu->FR.O) return;

    cpu->PC += inst->ops[0].displacement;
}