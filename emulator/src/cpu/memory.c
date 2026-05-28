#include "memory.h"

u8 read_byte(const CPU *cpu, const u16 address) {
    return cpu->memory[address];
}

u16 read_word(const CPU *cpu, const u16 address) {
    return cpu->memory[address] | cpu->memory[address + 1] << 8;
}

void write_byte(CPU *cpu, const u16 address, const u8 value) {
    cpu->memory[address] = value;
}

void write_word(CPU *cpu, const u16 address, const u16 value) {
    cpu->memory[address] = value & 0xFF;
    cpu->memory[address + 1] = value >> 8 & 0xFF;
}

u8 fetch_byte(CPU *cpu) {
    return read_byte(cpu, cpu->PC++);
}

u16 fetch_word(CPU *cpu) {
    cpu->PC += 2;
    return read_word(cpu, cpu->PC-2);
}