#include "../../inst.h"

void jnc_handler(CPU *cpu, Instruction *inst) {
    if (cpu->FR.C) return;

    cpu->PC += inst->ops[0].displacement;
}