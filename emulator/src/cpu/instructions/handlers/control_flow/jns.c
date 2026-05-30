#include "../../inst.h"

void jns_handler(CPU *cpu, Instruction *inst) {
    if (cpu->FR.N) return;

    cpu->PC += inst->ops[0].displacement;
}