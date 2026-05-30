#include "../../inst.h"

void jc_handler(CPU *cpu, Instruction *inst) {
    if (!cpu->FR.C) return;

    cpu->PC += inst->ops[0].displacement;
}