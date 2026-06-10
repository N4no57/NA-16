#ifndef NA_16_CPU_H
#define NA_16_CPU_H

#define MAX_BUFF_SIZE 100

#include "../machine.h"

typedef enum {
    DVZ,    // DiVide by Zero
    UO,     // Unknown Opcode
    PV,     // Privilege Violation
    PF,     // Page Fault
    GP,     // General Protection
} Exceptions;

typedef enum {
    OP_REG,
    OP_IMM,
    OP_REG_IND,
    OP_ABSOLUTE,
    OP_REG_IND_DISP,
    OP_SIB,
    OP_SIB_DISP,
} OperandMode;

typedef struct {
    OperandMode mode;
    struct {
        u16 reg;
        u16 immediate;
        i16 displacement;

        // SIB only
        u16 idx_reg;
        u8 scale;
    };
    u8 size;
} Operand;

typedef struct {
    u16 MEX;
    u8 AEX;
    bool has_escape_byte;
} Prefixes;

typedef struct Instruction {
    Prefixes prefixes;
    u8 opcode;
    Operand ops[3];
    u8 op_count;
    u64 size; // debugging metadata
} Instruction;

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

typedef void (*InstructionHandler)(CPU*, Instruction*);

typedef struct InstructionDef {
    const char *name;
    u8 operand_count;
    InstructionHandler handler;
    bool privileged;
} InstructionDef;

extern u64 interrupt_count;

void set_reg(CPU *cpu, u8 reg, u16 value);
u16 read_reg(const CPU *cpu, u8 reg);

void interrupt(CPU *cpu, Exceptions int_code);

void cpu_init(CPU *cpu);
void cpu_reset(CPU *cpu);
void execute_inst(CPU *cpu);
void execute(CPU *cpu);

#endif //NA_16_CPU_H
