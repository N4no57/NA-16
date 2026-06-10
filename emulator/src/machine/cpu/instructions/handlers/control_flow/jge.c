#include "../../inst.h"

void jge_handler(CPU *cpu, Instruction *inst) {
    if (cpu->FR.O != cpu->FR.N) return;

    cpu->PC += inst->ops[0].displacement;
}