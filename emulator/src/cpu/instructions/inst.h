#ifndef NA_16_INST_H
#define NA_16_INST_H

#include "../cpu.h"

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

void collect_operands(CPU *cpu, Instruction *inst, const u8 *inst_ops);
u16 operand_read(const CPU *cpu, Operand op);
void operand_write(CPU *cpu, Operand op, u16 value);

void set_flags(CPU *cpu, u32 value, const u32 values[2], u8 mask, u8 size);

InstructionDef *fetch_InstDef(Ops idx);
Instruction decode(CPU *cpu);

// Instruction handlers
// class 0: ALU ops
void add_handler(CPU *cpu, Instruction *inst);
void sub_handler(CPU *cpu, Instruction *inst);
void and_handler(CPU *cpu, Instruction *inst);
void or_handler(CPU *cpu, Instruction *inst);
void xor_handler(CPU *cpu, Instruction *inst);
void not_handler(CPU *cpu, Instruction *inst);

// class 1: data movement
void mov_handler(CPU *cpu, Instruction *inst);
void movsr_handler(CPU *cpu, Instruction *inst);
void movrs_handler(CPU *cpu, Instruction *inst);
void push_handler(CPU *cpu, Instruction *inst);
void pop_handler(CPU *cpu, Instruction *inst);
void lea_handler(CPU *cpu, Instruction *inst);

// class 2: control flow
void jmp_handler(CPU *cpu, Instruction *inst);

// class 3: system instructions

#endif //NA_16_INST_H
