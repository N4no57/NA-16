#ifndef NA_16_MACHINE_H
#define NA_16_MACHINE_H

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef struct CPU CPU;

typedef struct MMU {
    u16 kernel_page_table;
    u16 user_page_table;
} MMU;

typedef struct Memory Memory;

typedef struct InterruptController {
    u16 IVBR;
} InterruptController;

typedef struct {
    CPU cpu;

    Memory ram;
    MMU mmu;

    InterruptController PIC;

    bool powered_on;
} Machine;

#endif //NA_16_MACHINE_H
