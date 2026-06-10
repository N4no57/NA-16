#include "../../inst.h"

void jnz_handler(CPU *cpu, Instruction *inst) {
    if (cpu->FR.Z) return;

    cpu->PC += inst->ops[0].displacement;
}