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

typedef enum {
    // op table 1
    // class 0: ALU ops
    ADD = 0x00,
    SUB = 0x01,
    AND = 0x02,
    OR = 0x03,
    XOR = 0x04,
    NOT = 0x05,
    CMP = 0x06,
    TEST = 0x07,

    // class 1: data movement
    MOV = 0x10,
    MOVSR = 0x11,
    MOVRS = 0x12,
    PUSH = 0x13,
    POP = 0x14,
    LEA = 0x15,
    MOVS = 0x16,
    PUSHS = 0x17,
    POPS = 0x18,

    // class 2: control flow
    JMP = 0x20,
    JZ = 0x21, // JE = 0x21,
    JNZ = 0x22, // JNE = 0x22,
    JC = 0x23, // JB = 0x23,
    JNC = 0x24, // JAE = 0x24,
    JO = 0x25,
    JNO = 0x26,
    JS = 0x27,
    JNS = 0x28,
    JA = 0x29,
    JBE = 0x2A,
    JG = 0x2B,
    JGE = 0x2C,
    JL = 0x2D,
    JLE = 0x2E,
    RET = 0x2F,

    // class 3: system instructions
    NOP = 0x30,
    HLT = 0x31,

    // op table 2
    // class 0: ALU ops

    // class 1: data movement
    CALL = 0x120,

    // class 2: control flow

    // class 3: system instructions
} Ops;

bool is_cond_jump(const Instruction *inst);

void collect_operands(CPU *cpu, Instruction *inst, const u8 *inst_ops);
u16 operand_read(const CPU *cpu, Operand op);
void operand_write(CPU *cpu, Operand op, u16 value);

void set_flags(CPU *cpu, u32 value, const u32 values[2], u8 mask, u8 size);

InstructionDef *fetch_InstDef(Ops idx, bool has_escape_byte);
Instruction decode(CPU *cpu);

// Instruction handlers
// class 0: ALU ops
void add_handler(CPU *cpu, Instruction *inst);
void sub_handler(CPU *cpu, Instruction *inst);
void and_handler(CPU *cpu, Instruction *inst);
void or_handler(CPU *cpu, Instruction *inst);
void xor_handler(CPU *cpu, Instruction *inst);
void not_handler(CPU *cpu, Instruction *inst);
void cmp_handler(CPU *cpu, Instruction *inst);
void test_handler(CPU *cpu, Instruction *inst);

// class 1: data movement
void mov_handler(CPU *cpu, Instruction *inst);
void movsr_handler(CPU *cpu, Instruction *inst);
void movrs_handler(CPU *cpu, Instruction *inst);
void push_handler(CPU *cpu, Instruction *inst);
void pop_handler(CPU *cpu, Instruction *inst);
void lea_handler(CPU *cpu, Instruction *inst);
void movs_handler(CPU *cpu, Instruction *inst);
void pushs_handler(CPU *cpu, Instruction *inst);
void pops_handler(CPU *cpu, Instruction *inst);

// class 2: control flow
void jmp_handler(CPU *cpu, Instruction *inst);
void jz_handler(CPU *cpu, Instruction *inst);
void jnz_handler(CPU *cpu, Instruction *inst);
void jc_handler(CPU *cpu, Instruction *inst);
void jnc_handler(CPU *cpu, Instruction *inst);
void jo_handler(CPU *cpu, Instruction *inst);
void jno_handler(CPU *cpu, Instruction *inst);
void js_handler(CPU *cpu, Instruction *inst);
void jns_handler(CPU *cpu, Instruction *inst);
void ja_handler(CPU *cpu, Instruction *inst);
void jbe_handler(CPU *cpu, Instruction *inst);
void jg_handler(CPU *cpu, Instruction *inst);
void jge_handler(CPU *cpu, Instruction *inst);
void jl_handler(CPU *cpu, Instruction *inst);
void jle_handler(CPU *cpu, Instruction *inst);
void call_handler(CPU *cpu, Instruction *inst);
void ret_handler(CPU *cpu, Instruction *inst);

// class 3: system instructions

#endif //NA_16_INST_H
