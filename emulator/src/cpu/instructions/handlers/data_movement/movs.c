#include "../../inst.h"

void movs_handler(CPU *cpu, Instruction *inst) {
    inst->ops[1].reg += 0x40;
    const u16 source = operand_read(cpu, inst->ops[1]);

    inst->ops[0].reg += 0x40;
    operand_write(cpu, inst->ops[0], source);
}