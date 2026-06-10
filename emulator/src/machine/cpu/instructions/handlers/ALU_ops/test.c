#include "../../inst.h"

void test_handler(CPU *cpu, Instruction *inst) {
    const u16 l = operand_read(cpu, inst->ops[0]);
    const u16 r = operand_read(cpu, inst->ops[1]);

    const u16 result = l & r;

    set_flags(cpu, result, nullptr, 0b0011, inst->ops[0].size);
    cpu->FR.O = cpu->FR.C = 0;
}