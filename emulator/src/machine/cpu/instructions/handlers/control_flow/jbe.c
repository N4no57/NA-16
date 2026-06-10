#include "../../inst.h"

void jbe_handler(CPU *cpu, Instruction *inst) {
    if (!cpu->FR.Z && !cpu->FR.C) return;

    cpu->PC += inst->ops[0].displacement;
}