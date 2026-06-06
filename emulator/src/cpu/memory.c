#include "memory.h"

#include <stdlib.h>

typedef enum {
    PT_PRESENT = 1 << 11,
    PT_WRITABLE = 1 << 10,
    PT_EXECUTABLE = 1 << 9,
} PageTableFlags;

typedef struct {
    u32 frame;
} PageTableEntry;

u32 translate(const CPU *cpu, u32 vaddr) {
    PageTableEntry entry;
    if (cpu->FR.U == 1) {
        u32 *tmp = (u32 *)&cpu->memory[cpu->CR1 + (vaddr >> 12) * sizeof(u32)];
        entry.frame = *tmp;

        if ((entry.frame & PT_PRESENT) != PT_PRESENT) {
            // idk smth about a page fault
            return -1;
        }

        return entry.frame << 12 | vaddr & 0xFFF;
    }

    u32 *tmp = (u32 *)&cpu->memory[cpu->CR0 + (vaddr >> 12) * sizeof(u32)];
    entry.frame = *tmp;

    if ((entry.frame & PT_PRESENT) != PT_PRESENT) {
        // idk smth about a page fault
        exit(10);
    }

    return entry.frame & 0xFFFFF000 | vaddr & 0xFFF;
}

u8 read_byte(const CPU *cpu, const u16 address) {
    if (cpu->FR.V == 1) {
        return cpu->memory[translate(cpu, address)];
    }
    return cpu->memory[address];
}

u16 read_word(const CPU *cpu, const u16 address) {
    if (cpu->FR.V == 1) {
        u32 true_address = translate(cpu, address);
        return cpu->memory[true_address] | cpu->memory[true_address + 1] << 8;
    }
    return cpu->memory[address] | cpu->memory[address + 1] << 8;
}

void write_byte(CPU *cpu, const u16 address, const u8 value) {
    if (cpu->FR.V == 1) {
        cpu->memory[translate(cpu, address)] = value;
    } else {
        cpu->memory[address] = value;
    }
}

void write_word(CPU *cpu, const u16 address, const u16 value) {
    if (cpu->FR.V == 1) {
        u32 true_address = translate(cpu, address);
        cpu->memory[true_address] = value & 0xFF;
        cpu->memory[true_address + 1] = value >> 8 & 0xFF;
    } else {
        cpu->memory[address] = value & 0xFF;
        cpu->memory[address + 1] = value >> 8 & 0xFF;
    }
}

u8 fetch_byte(CPU *cpu) {
    return read_byte(cpu, cpu->PC++);
}

u16 fetch_word(CPU *cpu) {
    cpu->PC += 2;
    return read_word(cpu, cpu->PC-2);
}