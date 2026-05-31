#include "../../inst.h"
#include "../../../memory.h"

void call_handler(CPU *cpu, Instruction *inst) {
    write_byte(cpu, cpu->SP--, cpu->PC & 0xFF); // push return address
    write_byte(cpu, cpu->SP--, cpu->PC >> 8);

    const u16 address = operand_read(cpu, inst->ops[0]);

    cpu->PC = address;
}
