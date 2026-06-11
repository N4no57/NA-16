#include "../../inst.h"

bool js_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    if (cpu->sys.FR.N) cpu->sys.PC += inst->ops[0].displacement;
    return true;
}