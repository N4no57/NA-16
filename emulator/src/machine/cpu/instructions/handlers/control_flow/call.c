#include "../../inst.h"
#include "../../../../ram/memory.h"

void call_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    write_byte(machine, cpu->sys.SP--, cpu->sys.PC & 0xFF); // push return address
    write_byte(machine, cpu->sys.SP--, cpu->sys.PC >> 8);

    const u16 address = operand_read(machine, inst->ops[0]);

    cpu->sys.PC = address;
}
