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

typedef union {
    struct {
        u8 C : 1; // Carry
        u8 Z : 1; // Zero
        u8 O : 1; // Overflow
        u8 N : 1; // Negative
        u8 I : 1; // (Maskable) Interrupt enable
        u8 D : 1; // Debug
        u8 V : 1; // Virtual memory enable
        u8 U : 1; // User mode
    };
    u16 flags;
} flags;

typedef struct GPRegisters {
    u16 R[16];
} GPRegisters;

typedef struct SystemRegisters {
    u16 PC;
    u16 SP;
    u16 BP;
    u16 KSP;
    flags FR;
} SystemRegisters;

typedef struct CPU {
    GPRegisters gp;
    SystemRegisters sys;

    bool halt;
} CPU;

typedef struct MMU {
    u16 kernel_page_table;
    u16 user_page_table;
} MMU;

typedef struct Memory {
    u8 *memory;
    u64 memory_size;
} Memory;

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
