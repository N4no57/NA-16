#include "../../inst.h"
#include "../../../memory.h"

void ret_handler(CPU *cpu, Instruction *inst) {
    u16 address = read_byte(cpu, ++cpu->SP) << 8; // pop return address
    address |= read_byte(cpu, ++cpu->SP);

    cpu->PC = address;
}
