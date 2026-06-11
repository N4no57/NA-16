#include "cpu.h"
#include "../ram/memory.h"
#include "instructions/inst.h"

#include <string.h>

#include "../PIC/pic.h"

bool is_privileged_reg(u64 reg) {
    if (reg == 0x40) return true;
    if (reg > 0x42) return true;

    return false;
}

void set_reg(Machine *machine, const u8 reg, const u16 value) {
    CPU *cpu = &machine->cpu;
    if (reg < 0x40) {
        cpu->gp.R[reg] = value;
        return;
    }

    switch (reg) {
        case 0x40:
            cpu->sys.PC = value;
            break;
        case 0x41:
            cpu->sys.SP = value;
            break;
        case 0x42:
            cpu->sys.BP = value;
            break;
        case 0x43:
            cpu->sys.FR.flags = value;
            break;
        case 0x44:
            machine->mmu.kernel_page_table = value;
            break;
        case 0x45:
            machine->mmu.user_page_table = value;
            break;
        case 0x46:
            machine->PIC.IVBR = value;
            break;
        case 0x47:
            cpu->sys.KSP = value;
            break;
        default:
            break;
    }
}

u16 read_reg(const Machine *machine, const u8 reg) {
    const CPU *cpu = &machine->cpu;
    if (reg < 0x40) {
        return cpu->gp.R[reg];
    }

    switch (reg) {
        case 0x40:
            return cpu->sys.PC;
        case 0x41:
            return cpu->sys.SP;
        case 0x42:
            return cpu->sys.BP;
        case 0x43:
            return cpu->sys.FR.flags;
        case 0x44:
            return machine->mmu.kernel_page_table;
        case 0x45:
            return machine->mmu.user_page_table;
        case 0x46:
            return machine->PIC.IVBR;
        case 0x47:
            return cpu->sys.KSP;
        default:
            break;
    }
    return 0;
}

bool push_byte(Machine *machine, const u8 value) {
    const bool success = write_byte(machine, machine->cpu.sys.SP--, value);
    if (!success) return false;
    return true;
}

bool push_word(Machine *machine, const u16 value) {
    bool success = write_byte(machine, machine->cpu.sys.SP--, value & 0xFF);
    if (!success) return false;
    success = write_byte(machine, machine->cpu.sys.SP--, value >> 8);
    if (!success) return false;
    return true;
}

bool pop_byte(Machine *machine, u64 *value) {
    const bool success = read_byte(machine, ++machine->cpu.sys.SP, value);
    if (!success) return false;
    return true;
}

bool pop_word(Machine *machine, u64 *value) {
    bool success = read_byte(machine, ++machine->cpu.sys.SP, value);
    if (!success) return false;
    u64 tmp;
    success = read_byte(machine, ++machine->cpu.sys.SP, &tmp);
    if (!success) return false;
    *value = *value << 8 | tmp;
    return true;
}

bool iret(Machine *machine) {
    u64 tmp;
    bool success = pop_word(machine, &tmp);
    if (!success) return false;

    machine->cpu.sys.FR.flags = tmp;

    success = pop_word(machine, &tmp);
    if (!success) return false;

    machine->cpu.sys.FR.flags = tmp;

    return true;
}

void cpu_init(Machine *machine) {
    memset(machine->ram.memory, 0, machine->ram.memory_size);
    cpu_reset(machine);
}

void cpu_reset(Machine *machine) {
    CPU *cpu = &machine->cpu;
    memset(cpu->gp.R, 0, sizeof(machine->cpu.gp.R));
    cpu->sys.FR.flags = 0;
    cpu->sys.PC = 0xFFFE; // reset vec
    cpu->sys.SP = cpu->sys.BP = 0x1000;
    machine->mmu.kernel_page_table = machine->mmu.user_page_table = 0;
    cpu->sys.KSP = machine->PIC.IVBR = 0;
    cpu->halt = false;

    u64 tmp;
    read_word(machine, cpu->sys.PC, &tmp);
    cpu->sys.PC = tmp;
}

void execute_inst(Machine *machine) {
    CPU *cpu = &machine->cpu;
    if (cpu->halt) return;

    Instruction inst;
    bool success = decode(machine, &inst);

    if (!success) return;

    const InstructionDef *def = fetch_InstDef(inst.opcode, inst.prefixes.has_escape_byte);

    if (inst.opcode == NOP) return;
    if (inst.opcode == HLT) {
        cpu->halt = true;
        return;
    }

    if (!def->handler) {
        // interrupt(cpu, UO);
    }

    success = def->handler(machine, &inst);
    if (!success) return;
}

void cpu_step(Machine *machine) {
    if (machine->PIC.exception.pending) {
        enter_exception(machine);
        return;
    }

    if (machine->cpu.sys.FR.I) {
        // interrupts are not masked
        if (machine->PIC.interrupt_requests.read_ptr != machine->PIC.interrupt_requests.write_ptr) {
            enter_irq(machine);
            return;
        }
    }

    execute_inst(machine);
}
