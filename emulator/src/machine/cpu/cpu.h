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

typedef bool (*InstructionHandler)(Machine*, Instruction*);

typedef struct InstructionDef {
    const char *name;
    u8 operand_count;
    InstructionHandler handler;
    bool privileged;
} InstructionDef;

bool is_privileged_reg(u64 reg);

void set_reg(Machine *machine, u8 reg, u16 value);
u16 read_reg(const Machine *machine, u8 reg);

bool push_byte(Machine *machine, u8 value);
bool push_word(Machine *machine, u16 value);
bool pop_byte(Machine *machine, u64 *value);
bool pop_word(Machine *machine, u64 *value);

void cpu_init(Machine *machine);
void cpu_reset(Machine *machine);
void execute_inst(Machine *machine);

#endif //NA_16_CPU_H
