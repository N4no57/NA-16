#ifndef NA_16_DEBUGGER_H
#define NA_16_DEBUGGER_H

typedef struct CPU CPU;
typedef struct Instruction Instruction;
typedef struct InstructionDef InstructionDef;

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#define MAX_BREAKPOINTS 64

extern u32 breakpoints[64];

void print_instruction(const CPU *cpu, const Instruction *inst, const InstructionDef *def);

void commands(bool *halt);

bool add_breakpoint(u16 address);
bool remove_breakpoint(u16 address);
bool check_breakpoint(CPU *cpu);

#endif //NA_16_DEBUGGER_H
