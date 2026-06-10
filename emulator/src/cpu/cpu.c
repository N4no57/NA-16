#include "cpu.h"
#include "memory.h"
#include "instructions/inst.h"

#include <stdlib.h>
#include <string.h>

void set_reg(CPU *cpu, const u8 reg, const u16 value) {
    switch (reg) {
        case 0x0: // R0
            cpu->R0 = value;
            break;
        case 0x1: // R1
            cpu->R1 = value;
            break;
        case 0x2: // R2
            cpu->R2 = value;
            break;
        case 0x3: // R3
            cpu->R3 = value;
            break;
        case 0x4:
            cpu->R4 = value;
            break;
        case 0x5:
            cpu->R5 = value;
            break;
        case 0x6:
            cpu->R6 = value;
            break;
        case 0x7:
            cpu->R7 = value;
            break;
        case 0x40:
            cpu->PC = value;
            break;
        case 0x41:
            cpu->SP = value;
            break;
        case 0x42:
            cpu->BP = value;
            break;
        case 0x43:
            cpu->FR.flags = value;
            break;
        case 0x44:
            cpu->CR0 = value;
            break;
        case 0x45:
            cpu->CR1 = value;
            break;
        case 0x46:
            cpu->IVBR = value;
            break;
        case 0x47:
            cpu->KSP = value;
            break;
        default:
            break;
    }
}

u16 read_reg(const CPU *cpu, const u8 reg) {
    switch (reg) {
        case 0x0: // R0
            return cpu->R0;
        case 0x1: // R1
            return cpu->R1;
        case 0x2: // R2
            return cpu->R2;
        case 0x3: // R3
            return cpu->R3;
        case 0x4:
            return cpu->R4;
        case 0x5:
            return cpu->R5;
        case 0x6:
            return cpu->R6;
        case 0x7:
            return cpu->R7;
        case 0x40:
            return cpu->PC;
        case 0x41:
            return cpu->SP;
        case 0x42:
            return cpu->BP;
        case 0x43:
            return cpu->FR.flags;
        case 0x44:
            return cpu->CR0;
        case 0x45:
            return cpu->CR1;
        case 0x46:
            return cpu->IVBR;
        case 0x47:
            return cpu->KSP;
        default:
            break;
    }
    return 0;
}

void push_byte(CPU *cpu, u8 value) {
    write_byte(cpu, cpu->SP--, value);
}

void push_word(CPU *cpu, u16 value) {
    write_byte(cpu, cpu->SP--, value & 0xFF);
    write_byte(cpu, cpu->SP--, value >> 8);
}

u8 pop_byte(CPU *cpu) {
    return read_byte(cpu, ++cpu->SP);
}

u16 pop_word(CPU *cpu) {
    u16 ret_val = read_byte(cpu, ++cpu->SP) << 8;
    ret_val |= read_byte(cpu, ++cpu->SP);
    return ret_val;
}

u64 interrupt_count = 0;

void interrupt(CPU *cpu, Exceptions int_code) {
    if (int_code >= 0x20 && cpu->FR.I == 0) return; // maskable interrupt to ignore it

    if (cpu->FR.U) {
        // userland is more complex
        const u16 tmp = cpu->SP;
        cpu->SP = cpu->KSP;
        cpu->KSP = tmp;
        push_word(cpu, cpu->PC);
        push_word(cpu, cpu->FR.flags);
        cpu->FR.U = 0;

        u16 IVTB = cpu->IVBR;
        u16 address = cpu->memory[IVTB+int_code*2];
        cpu->PC = address;
    } else {
        push_word(cpu, cpu->PC);
        push_word(cpu, cpu->FR.flags);

        u16 IVTB = cpu->IVBR;
        u16 address = cpu->memory[IVTB+int_code*2];
        cpu->PC = address;
    }

    cpu->halt = false;
    interrupt_count++;
}

void iret(CPU *cpu) {
    cpu->FR.flags = pop_word(cpu);
    cpu->PC = pop_word(cpu);
}

void cpu_init(CPU *cpu) {
    memset(cpu->memory, 0, cpu->memory_size);
    cpu_reset(cpu);
}

void cpu_reset(CPU *cpu) {
    cpu->R0 = cpu->R1 = cpu->R2 = cpu->R3 = 0;
    cpu->R4 = cpu->R5 = cpu->R6 = cpu->R7 = 0;
    cpu->PC =0xFFFE; // reset vec
    cpu->SP = cpu->BP = 0x1000;
    cpu->PC = read_word(cpu, cpu->PC);
    cpu->CR0 = cpu->CR1 = 0;
    cpu->KSP = cpu->IVBR = 0;
    cpu->halt = false;
}

void execute_inst(CPU *cpu) {
    Instruction inst = decode(cpu);

    const InstructionDef *def = fetch_InstDef(inst.opcode, inst.prefixes.has_escape_byte);

    if (inst.opcode == NOP) return;
    if (inst.opcode == HLT) {
        cpu->halt = true;
        return;
    }

    if (!def->handler) {
        interrupt(cpu, UO);
    }

    def->handler(cpu, &inst);
}

void execute(CPU *cpu) {
    while (1) {
        execute_inst(cpu);
        if (cpu->halt) break;
    }
}
