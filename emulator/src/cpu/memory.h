#ifndef NA_16_MEMORY_H
#define NA_16_MEMORY_H

#include "cpu.h"

u8 read_byte(const CPU *cpu, u16 address);
u16 read_word(const CPU *cpu, u16 address);

void write_byte(CPU *cpu, u16 address, u8 value);
void write_word(CPU *cpu, u16 address, u16 value);

u8 fetch_byte(CPU *cpu);
u16 fetch_word(CPU *cpu);

#endif //NA_16_MEMORY_H
