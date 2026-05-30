#include "../../inst.h"

void jo_handler(CPU *cpu, Instruction *inst) {
    if (!cpu->FR.O) return;

    cpu->PC += inst->ops[0].displacement;
}