#ifndef NA_16_MEMORY_H
#define NA_16_MEMORY_H

#include "../machine.h"

bool read_byte(Machine *machine, u16 address, u64 *value);
bool read_word(const Machine *machine, u64 address, u64 *value);

bool write_byte(Machine *machine, u16 address, u8 value);
bool write_word(const Machine *machine, u16 address, u16 value);

bool fetch_byte(Machine *machine, u64 *value);
bool fetch_word(Machine *machine, u64 *value);

#endif //NA_16_MEMORY_H
