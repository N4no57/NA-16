#include "../../inst.h"
#include "../../../../PIC/pic.h"

bool int_handler(Machine *machine, Instruction *inst) {
    u64 interrupt_code;
    if (!operand_read(machine, inst->ops[0], &interrupt_code)) return false;

    pic_raise(&machine->PIC, interrupt_code);

    return true;
}
