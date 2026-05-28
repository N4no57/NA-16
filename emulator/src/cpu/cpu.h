#ifndef NA_16_CPU_H
#define NA_16_CPU_H

#define MAX_BUFF_SIZE 100

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
    // class 0: ALU ops
    ADD = 0x00,
    SUB = 0x01,
    AND = 0x02,
    OR = 0x03,
    XOR = 0x04,
    NOT = 0x05,

    // class 1: data movement
    MOV = 0x10,
    MOVSR = 0x11,
    MOVRS = 0x12,
    PUSH = 0x13,
    POP = 0x14,
    LEA = 0x15,

    // class 2: control flow
    JMP = 0x20,

    // class 3: system instructions
    NOP = 0x30,
    HLT = 0x31
} Ops;

typedef enum {
    OP_REG,
    OP_IMM,
    OP_REG_IND
} OperandMode;

typedef struct {
    OperandMode mode;
    struct {
        u8 reg;
        u16 immediate;
    };
    u8 size;
} Operand;

typedef struct {
    u16 MEX;
    u8 AEX;
} Prefixes;

typedef struct {
    Prefixes prefixes;
    u8 opcode;
    Operand ops[3];
    u8 op_count;
} Instruction;

#define MEMORY_SIZE 0x10000

typedef struct {
    u16 PC;
    u16 SP;
    u16 BP;
    u16 R0, R1, R2, R3, R4, R5, R6, R7;

    u8 memory[MEMORY_SIZE];
} CPU;

typedef void (*InstructionHandler)(CPU*, Instruction*);

typedef struct {
    const char *name;
    u8 operand_count;
    InstructionHandler handler;
} InstructionDef;

void set_reg(CPU *cpu, u16 reg, u16 value);
u16 read_reg(const CPU *cpu, u16 reg);

void cpu_init(CPU *cpu);
void cpu_reset(CPU *cpu);
void execute(CPU *cpu);

#endif //NA_16_CPU_H
