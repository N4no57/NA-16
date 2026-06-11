#include "../../inst.h"

bool jno_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    if (!cpu->sys.FR.O) cpu->sys.PC += inst->ops[0].displacement;
    return true;
}