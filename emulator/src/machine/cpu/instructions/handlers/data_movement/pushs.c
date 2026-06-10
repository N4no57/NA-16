#include "../../inst.h"
#include "../../../../ram/memory.h"

void pushs_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    inst->ops[0].reg += 0x40;
    if (inst->ops[0].mode == OP_REG) {
        if (inst->ops[0].reg >= 0x43 && cpu->sys.FR.U) // interrupt(machine, PV); // any special purpose register that isn't
        if (inst->ops[0].reg == 0x40 && cpu->sys.FR.U) // interrupt(machine, PV); // SP or BP causes a privilege violation
        return;
    }
    const u16 source = operand_read(machine, inst->ops[0]);

    if (inst->ops[0].size == 2) {
        write_byte(machine, cpu->sys.SP--, source & 0xFF);
        write_byte(machine, cpu->sys.SP--, source >> 8);
    } else if (inst->ops[0].size == 1) {
        write_byte(machine, cpu->sys.SP--, source & 0xFF);
    }
}