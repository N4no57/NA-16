#include "../../inst.h"
#include "../../../../ram/memory.h"

void pushs_handler(CPU *cpu, Instruction *inst) {
    inst->ops[0].reg += 0x40;
    if (inst->ops[0].mode == OP_REG) {
        if (inst->ops[0].reg >= 0x43 && cpu->FR.U) interrupt(cpu, PV); // any special purpose register that isn't
        if (inst->ops[0].reg == 0x40 && cpu->FR.U) interrupt(cpu, PV); // SP or BP causes a privilege violation
        return;
    }
    const u16 source = operand_read(cpu, inst->ops[0]);

    if (inst->ops[0].size == 2) {
        write_byte(cpu, cpu->SP--, source & 0xFF);
        write_byte(cpu, cpu->SP--, source >> 8);
    } else if (inst->ops[0].size == 1) {
        write_byte(cpu, cpu->SP--, source & 0xFF);
    }
}