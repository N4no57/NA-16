#include "cpu.h"
#include "../ram/memory.h"
#include "instructions/inst.h"

#include <stdlib.h>
#include <string.h>

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

void push_byte(Machine *machine, u8 value) {
    write_byte(machine, machine->cpu.sys.SP--, value);
}

void push_word(Machine *machine, u16 value) {
    write_byte(machine, machine->cpu.sys.SP--, value & 0xFF);
    write_byte(machine, machine->cpu.sys.SP--, value >> 8);
}

u8 pop_byte(Machine *machine) {
    return read_byte(machine, ++machine->cpu.sys.SP);
}

u16 pop_word(Machine *machine) {
    u16 ret_val = read_byte(machine, ++machine->cpu.sys.SP) << 8;
    ret_val |= read_byte(machine, ++machine->cpu.sys.SP);
    return ret_val;
}

void iret(Machine *machine) {
    machine->cpu.sys.FR.flags = pop_word(machine);
    machine->cpu.sys.PC = pop_word(machine);
}

void cpu_init(Machine *machine) {
    memset(machine->ram.memory, 0, machine->ram.memory_size);
    cpu_reset(machine);
}

void cpu_reset(Machine *machine) {
    CPU *cpu = &machine->cpu;
    memset(cpu->gp.R, 0, sizeof(machine->cpu.gp.R));
    cpu->sys.FR.flags = 0;
    cpu->sys.PC =0xFFFE; // reset vec
    cpu->sys.SP = cpu->sys.BP = 0x1000;
    cpu->sys.PC = read_word(machine, cpu->sys.PC);
    machine->mmu.kernel_page_table = machine->mmu.user_page_table = 0;
    cpu->sys.KSP = machine->PIC.IVBR = 0;
    cpu->halt = false;
}

void execute_inst(Machine *machine) {
    CPU *cpu = &machine->cpu;
    if (cpu->halt) return;

    Instruction inst;
    const bool success = decode(machine, &inst);

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

    def->handler(machine, &inst);
}
