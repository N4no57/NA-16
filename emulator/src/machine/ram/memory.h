#ifndef NA_16_MEMORY_H
#define NA_16_MEMORY_H

#include "../machine.h"

u8 read_byte(Machine *machine, u16 address);
u16 read_word(Machine *machine, u16 address);

void write_byte(Machine *machine, u16 address, u8 value);
void write_word(Machine *machine, u16 address, u16 value);

u8 fetch_byte(Machine *machine);
u16 fetch_word(Machine *machine);

#endif //NA_16_MEMORY_H
