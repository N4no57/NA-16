#ifndef NA_16_ASMLIB_H
#define NA_16_ASMLIB_H

#define MAXTEMPSIZE 100

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef enum registers {
    // general registers
    R0, R1, R2, R3, R4, R5, R6, R7,
    // special registers
    PC, SP, BP, FR, CR0, CR1, IVBR, KSP,
    NONE
} registers_t;

typedef enum {
    SS_BYTE = 1,
    SS_WORD = 2,
    SS_NONE
} SizeSpecs;

typedef struct Position {
    u8 *filename;
    u8 *source;
    u64 idx;
    u64 line;
    u64 column;
} Position;

typedef enum operand_types {
    REGISTER,
    IMMEDIATE,
    REG_INDIRECT,
    ABSOLUTE,
    REG_IND_DISP,
    SIB,
    SIB_DISP,

    // special operand types
    DISPLACEMENT,
    SYMBOL
} operand_types;

typedef struct bytes {
    u8 *data;
    u64 count;
    u64 size;
} bytes;

#define MAX_OPERANDS 3

typedef enum {
    CLASS_DEST,
    CLASS_SOURCE,
    CLASS_REGISTER_ONLY,
    CLASS_DISP,
    CLASS_IMM,
    CLASS_REG_AND_IMM
} OperandClass;

typedef struct {
    i32 operand_count;
    OperandClass classes[MAX_OPERANDS];
} OperandPattern;

#define MAX_SIGNATURES 64

typedef struct InstructionSpec {
    char *mnemonic;
    u8 class;
    u8 opcode;
    OperandPattern operand_pattern;
} InstructionSpec;

InstructionSpec get_spec(const char *mnemonic);

void toUpper(u8 *str);
void toLower(u8 *str);

bool is_cond_jump(const char *mnemonic);
bool ismnemonic(u8 *string);
bool isregister(const u8 *string);
bool isSPR(registers_t reg);
bool issizespec(const u8 *string);
bool operand_matches_class(operand_types type, OperandClass cls, bool is_cond_jump);

i64 getregister(const u8 *string);
i64 getsizespec(const u8 *string);

char *read_assembly(const char *file);

#endif //NA_16_ASMLIB_H
