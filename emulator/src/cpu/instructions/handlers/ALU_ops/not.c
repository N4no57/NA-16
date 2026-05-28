#include "../../inst.h"

void not_handler(CPU *cpu, Instruction *inst) {
    const u16 l = operand_read(cpu, inst->ops[1]);

    const u16 result = ~l;

    operand_write(cpu, inst->ops[0], result);
    set_flags(cpu, result, nullptr, 0b0011, inst->ops[0].size);
}