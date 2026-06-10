#include "../../inst.h"
#include "../../../memory.h"

void push_handler(CPU *cpu, Instruction *inst) {
    const u16 source = operand_read(cpu, inst->ops[0]);

    if (inst->ops[0].size == 2) {
        write_byte(cpu, cpu->SP--, source & 0xFF);
        write_byte(cpu, cpu->SP--, source >> 8);
    } else if (inst->ops[0].size == 1) {
        write_byte(cpu, cpu->SP--, source & 0xFF);
    }
}