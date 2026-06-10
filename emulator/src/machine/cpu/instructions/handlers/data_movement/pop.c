#include "../../inst.h"
#include "../../../../ram/memory.h"

void pop_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    u16 source = 0;

    if (inst->ops[0].size == 2) {
        source = read_byte(machine, ++cpu->sys.SP) << 8;
        source |= read_byte(machine, ++cpu->sys.SP);
    } else if (inst->ops[0].size == 1) {
        source = read_byte(machine, ++cpu->sys.SP);
    }

    operand_write(machine, inst->ops[0], source);
}