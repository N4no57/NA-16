#include "../../inst.h"

void jmp_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    const u16 address = operand_read(machine, inst->ops[0]);

    cpu->sys.PC = address;
}