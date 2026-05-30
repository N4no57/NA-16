#include "../../inst.h"

void jg_handler(CPU *cpu, Instruction *inst) {
    if (!cpu->FR.Z || cpu->FR.O != cpu->FR.N) return;

    cpu->PC += inst->ops[0].displacement;
}