#include "../../inst.h"

void or_handler(Machine *machine, Instruction *inst)  {
    CPU *cpu = &machine->cpu;
    const u16 l = operand_read(machine, inst->ops[1]);
    const u16 r = operand_read(machine, inst->ops[2]);

    const u16 result = l | r;

    operand_write(machine, inst->ops[0], result);
    
    set_flags(machine, result, nullptr, 0b0011, inst->ops[0].size);
    cpu->sys.FR.O = cpu->sys.FR.C = 0;
}