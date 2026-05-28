#include "../../inst.h"
#include "../../../memory.h"

void pop_handler(CPU *cpu, Instruction *inst) {
    u16 source = 0;

    if (inst->ops[0].size == 2) {
        source = read_byte(cpu, cpu->SP++) << 8;
        source |= read_byte(cpu, cpu->SP++);
        read_byte(cpu, cpu->SP++);
    } else if (inst->ops[0].size == 1) {
        source = read_byte(cpu, cpu->SP++);
    }

    operand_write(cpu, inst->ops[0], source);
}