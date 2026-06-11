#ifndef NA_16_INST_H
#define NA_16_INST_H

#include "../cpu.h"

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

bool collect_operands(Machine *machine, Instruction *inst, const u8 *inst_ops);
bool operand_read(Machine *machine, Operand op, u64 *value);
bool operand_write(Machine *machine, Operand op, u16 value);

void set_flags(Machine *machine, u32 value, const u32 values[2], u8 mask, u8 size);

InstructionDef *fetch_InstDef(Ops idx, bool has_escape_byte);
bool decode(Machine *machine, Instruction *inst);

// Instruction handlers
// class 0: ALU ops
bool add_handler(Machine *machine, Instruction *inst);
bool sub_handler(Machine *machine, Instruction *inst);
bool and_handler(Machine *machine, Instruction *inst);
bool or_handler(Machine *machine, Instruction *inst);
bool xor_handler(Machine *machine, Instruction *inst);
bool not_handler(Machine *machine, Instruction *inst);
bool cmp_handler(Machine *machine, Instruction *inst);
bool test_handler(Machine *machine, Instruction *inst);

// class 1: data movement
bool mov_handler(Machine *machine, Instruction *inst);
bool movsr_handler(Machine *machine, Instruction *inst);
bool movrs_handler(Machine *machine, Instruction *inst);
bool push_handler(Machine *machine, Instruction *inst);
bool pop_handler(Machine *machine, Instruction *inst);
bool lea_handler(Machine *machine, Instruction *inst);
bool movs_handler(Machine *machine, Instruction *inst);
bool pushs_handler(Machine *machine, Instruction *inst);
bool pops_handler(Machine *machine, Instruction *inst);

// class 2: control flow
bool jmp_handler(Machine *machine, Instruction *inst);
bool jz_handler(Machine *machine, Instruction *inst);
bool jnz_handler(Machine *machine, Instruction *inst);
bool jc_handler(Machine *machine, Instruction *inst);
bool jnc_handler(Machine *machine, Instruction *inst);
bool jo_handler(Machine *machine, Instruction *inst);
bool jno_handler(Machine *machine, Instruction *inst);
bool js_handler(Machine *machine, Instruction *inst);
bool jns_handler(Machine *machine, Instruction *inst);
bool ja_handler(Machine *machine, Instruction *inst);
bool jbe_handler(Machine *machine, Instruction *inst);
bool jg_handler(Machine *machine, Instruction *inst);
bool jge_handler(Machine *machine, Instruction *inst);
bool jl_handler(Machine *machine, Instruction *inst);
bool jle_handler(Machine *machine, Instruction *inst);
bool call_handler(Machine *machine, Instruction *inst);
bool ret_handler(Machine *machine, Instruction *inst);

// class 3: system instructions

#endif //NA_16_INST_H
