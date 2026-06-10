#include "../../inst.h"

void or_handler(CPU *cpu, Instruction *inst)  {
    const u16 l = operand_read(cpu, inst->ops[1]);
    const u16 r = operand_read(cpu, inst->ops[2]);

    const u16 result = l | r;

    operand_write(cpu, inst->ops[0], result);
    
    set_flags(cpu, result, nullptr, 0b0011, inst->ops[0].size);
    cpu->FR.O = cpu->FR.C = 0;
}