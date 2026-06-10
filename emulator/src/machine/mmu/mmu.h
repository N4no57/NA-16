#ifndef NA_16_MMU_H
#define NA_16_MMU_H

#include "../machine.h"

typedef enum {
    PT_PRESENT = 1 << 11,
    PT_WRITABLE = 1 << 10,
    PT_EXECUTABLE = 1 << 9,
    PT_USER_ACCESS = 1 << 8
} PageTableFlags;

typedef struct {
    u32 frame;
} PageTableEntry;

u32 translate(Machine *machine, u32 vaddr);
void is_executable(Machine *machine);

#endif //NA_16_MMU_H
