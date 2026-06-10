#include "../../inst.h"
#include "../../../../ram/memory.h"

void ret_handler(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    u16 address = read_byte(machine, ++cpu->sys.SP) << 8; // pop return address
    address |= read_byte(machine, ++cpu->sys.SP);

    cpu->sys.PC = address;
}
