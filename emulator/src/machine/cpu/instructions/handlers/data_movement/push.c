#include "../../inst.h"
#include "../../../../ram/memory.h"

void push_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    const u16 source = operand_read(machine, inst->ops[0]);

    if (inst->ops[0].size == 2) {
        write_byte(machine, cpu->sys.SP--, source & 0xFF);
        write_byte(machine, cpu->sys.SP--, source >> 8);
    } else if (inst->ops[0].size == 1) {
        write_byte(machine, cpu->sys.SP--, source & 0xFF);
    }
}