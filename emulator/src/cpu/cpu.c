#include "cpu.h"
#include "memory.h"
#include "instructions/inst.h"

#include <stdio.h>
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
        default:
            break;
    }
    return 0;
}

void cpu_init(CPU *cpu) {
    memset(cpu->memory, 0, sizeof(cpu->memory));
    cpu_reset(cpu);
}

void cpu_reset(CPU *cpu) {
    cpu->R0 = cpu->R1 = cpu->R2 = cpu->R3 = 0;
    cpu->R4 = cpu->R5 = cpu->R6 = cpu->R7 = 0;
    cpu->PC =0xFFFE; // reset vec
    cpu->SP = cpu->BP = 0x1000;
    cpu->PC = read_word(cpu, cpu->PC);
}

void execute(CPU *cpu) {
    Instruction inst = decode(cpu);

    const InstructionDef *def = fetch_InstDef(inst.opcode);

    if (!def->handler) {
        fprintf(stderr, "Excuse me what the actual fuck are you doing in my house?\n");
        exit(EXIT_FAILURE);
    }

    def->handler(cpu, &inst);
}
