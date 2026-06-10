#include "../../inst.h"

void jo_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    if (!cpu->sys.FR.O) return;

    cpu->sys.PC += inst->ops[0].displacement;
}