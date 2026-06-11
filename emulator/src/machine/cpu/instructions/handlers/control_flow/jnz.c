#include "../../inst.h"

bool jnz_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    if (!cpu->sys.FR.Z) cpu->sys.PC += inst->ops[0].displacement;
    return true;
}