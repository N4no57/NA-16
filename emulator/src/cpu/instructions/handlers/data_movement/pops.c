#include "../../inst.h"
#include "../../../memory.h"

void pops_handler(CPU *cpu, Instruction *inst) {
    u16 source = 0;

    if (inst->ops[0].size == 2) {
        source = read_byte(cpu, ++cpu->SP) << 8;
        source |= read_byte(cpu, ++cpu->SP);
    } else if (inst->ops[0].size == 1) {
        source = read_byte(cpu, ++cpu->SP);
    }

    inst->ops[0].reg += 0x40;
    operand_write(cpu, inst->ops[0], source);
}