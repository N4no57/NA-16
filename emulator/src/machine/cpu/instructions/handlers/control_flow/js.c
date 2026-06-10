#include "../../inst.h"

void js_handler(CPU *cpu, Instruction *inst) {
    if (!cpu->FR.N) return;

    cpu->PC += inst->ops[0].displacement;
}