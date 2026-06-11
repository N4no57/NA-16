#include "pic.h"

#include "../cpu/cpu.h"

void raise_exception(Machine *machine, const Exceptions code, const u64 address) {
    machine->PIC.exception.pending = true;
    machine->PIC.exception.fault_address = address;
    machine->PIC.exception.vector = machine->ram.memory[machine->PIC.IVBR + code * 2];
}

void pic_raise(InterruptController *pic, const u8 code) {
    IRQs *interrupts = &pic->interrupt_requests;
    interrupts->pending[interrupts->write_ptr] = code;
    interrupts->write_ptr++;
}

u64 get_irq_vector(const Machine *machine, const u8 code) {
    return machine->ram.memory[machine->PIC.IVBR + code * 2];
}

void interrupt_prelude(Machine *machine) {
    CPU *cpu = &machine->cpu;

    if (cpu->sys.FR.U) {
        const u16 tmp = cpu->sys.SP;
        cpu->sys.SP = cpu->sys.KSP;
        cpu->sys.KSP = tmp;
        cpu->sys.FR.U = 0;
    }

    push_word(machine, cpu->sys.PC);
    push_word(machine, cpu->sys.FR.flags);

    cpu->sys.FR.I = 0; // mask interrupts
}

void enter_exception(Machine *machine) {
    CPU *cpu = &machine->cpu;

    interrupt_prelude(machine);

    cpu->sys.PC = machine->PIC.exception.vector;
}

void enter_irq(Machine *machine) {
    CPU *cpu = &machine->cpu;

    interrupt_prelude(machine);

    const u8 interrupt = machine->PIC.interrupt_requests.pending[machine->PIC.interrupt_requests.read_ptr];
    machine->PIC.interrupt_requests.read_ptr++;

    cpu->sys.PC = get_irq_vector(machine, interrupt);
}