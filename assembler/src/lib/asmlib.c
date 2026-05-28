#include "asmlib.h"
#include "../parser/ast.h"

#include <string.h>

char *registers[] = {
    "R0", // 0
    "R1", // 1
    "R2", // 2
    "R3", // 3
    "R4", // 4
    "R5", // 5
    "R6", // 6
    "R7", // 7

    "PC", // 8
    "SP", // 9
    "BP", // 10
    "NAN", // 11
    "NAN", // 12
    "NAN", // 13
    "NAN", // 14
    "NAN" // 15
};

char *size_specs[] = {
    [SS_BYTE] = "BYTE",
    [SS_WORD] = "WORD"
};

InstructionSpec ISA[] = {
    {
        "ADD",
        0,
        0,
        {
            {3, {REGISTER, REGISTER, REGISTER}},
            {3, {REGISTER, IMMEDIATE, REGISTER}},
            {3, {REGISTER, REGISTER, IMMEDIATE}},
            {3, {REGISTER, IMMEDIATE, IMMEDIATE}},
            {3, {REGISTER, REGISTER, REG_INDIRECT}},
            {3, {REGISTER, REG_INDIRECT, REGISTER}},
            {3, {REGISTER, REG_INDIRECT, REG_INDIRECT}},
            {3, {REGISTER, IMMEDIATE, REG_INDIRECT}},
            {3, {REGISTER, REG_INDIRECT, IMMEDIATE}},
            {3, {REG_INDIRECT, REGISTER, REGISTER}},
            {3, {REG_INDIRECT, IMMEDIATE, REGISTER}},
            {3, {REG_INDIRECT, REGISTER, IMMEDIATE}},
            {3, {REG_INDIRECT, IMMEDIATE, IMMEDIATE}},
        },
        49-36
    },
    {
        "SUB",
        0,
        1,
        {
            {3, {REGISTER, REGISTER, REGISTER}},
            {3, {REGISTER, IMMEDIATE, REGISTER}},
            {3, {REGISTER, REGISTER, IMMEDIATE}},
            {3, {REGISTER, IMMEDIATE, IMMEDIATE}},
            {3, {REGISTER, REGISTER, REG_INDIRECT}},
            {3, {REGISTER, REG_INDIRECT, REGISTER}},
            {3, {REGISTER, REG_INDIRECT, REG_INDIRECT}},
            {3, {REGISTER, IMMEDIATE, REG_INDIRECT}},
            {3, {REGISTER, REG_INDIRECT, IMMEDIATE}},
            {3, {REG_INDIRECT, REGISTER, REGISTER}},
            {3, {REG_INDIRECT, IMMEDIATE, REGISTER}},
            {3, {REG_INDIRECT, REGISTER, IMMEDIATE}},
            {3, {REG_INDIRECT, IMMEDIATE, IMMEDIATE}},
        },
        70-57
    },
    {
        "MOV",
        1,
        0,
        {
            {2, {REGISTER, REGISTER}},
            {2, {REGISTER, IMMEDIATE}},
            {2, {REGISTER, REG_INDIRECT}},
            {2, {REG_INDIRECT, REGISTER}},
            {2, {REG_INDIRECT, IMMEDIATE}},
        }
    },
        {
        "JMP",
        2,
        0,
        {1,{

            }
        }
    }
};

InstructionSpec get_spec(const char *mnemonic) {
    constexpr u64 size = sizeof(ISA) / sizeof(ISA[0]);

    char buff[MAXTEMPSIZE];
    strcpy(buff, mnemonic);
    toUpper((u8 *)buff);

    for (u64 i = 0; i < size; i++) {
        if (strcmp(ISA[i].mnemonic, buff) == 0) {
            return ISA[i];
        }
    }
    InstructionSpec ret = {0};
    ret.signature_count = -1;
    return ret;
}

i32 match_signature(const NodeInstruction* inst, const InstructionSignature* sig) {
    if (inst->operand_count != sig->operand_count) return 0;

    for (i32 i = 0; i < inst->operand_count; i++) {
        if (inst->operands[i].kind != sig->kinds[i]) return 0;
    }

    return 1;
}

void toUpper(u8 *str) {
    for (int i = 0; i < strlen((char *)str); i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] -= 32;
        }
    }
}

void toLower(u8 *str) {
    for (int i = 0; i < strlen((char *)str); i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] += 32;
        }
    }
}

u64 ismnemonic(u8 *string) {
    InstructionSpec spec = get_spec((char *)string);
    if (spec.signature_count == -1) {
        return 0;
    }
    return 1;
}

u64 isregister(const u8 *string) {
    u8 tmp[MAXTEMPSIZE];
    strcpy((char *)tmp, (char *)string);
    toUpper(tmp);
    for (int i = 0; i < sizeof(registers) / sizeof(registers[0]); i++) {
        if (strcmp((char *)tmp, registers[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

u64 issizespec(const u8 *string) {
    u8 tmp[MAXTEMPSIZE];
    strcpy((char *)tmp, (char *)string);
    toUpper(tmp);
    for (int i = 0; i < sizeof(size_specs) / sizeof(size_specs[0]); i++) {
        if (size_specs[i] == nullptr) continue;
        if (strcmp((char *)tmp, size_specs[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

i64 getmnemonic(u8 *string) {
    constexpr u64 size = sizeof(ISA) / sizeof(ISA[0]);

    char buff[MAXTEMPSIZE];
    strcpy(buff, (char *)string);
    toUpper((u8 *)buff);

    for (i64 i = 0; i < size; i++) {
        if (strcmp(ISA[i].mnemonic, buff) == 0) {
            return i;
        }
    }
    return -1;
}

i64 getregister(const u8 *string) {
    u8 tmp[MAXTEMPSIZE];
    strcpy((char *)tmp, (char *)string);
    toUpper(tmp);
    for (int i = 0; i < sizeof(registers) / sizeof(registers[0]); i++) {
        if (strcmp((char *)tmp, registers[i]) == 0) {
            return i;
        }
    }
    return NONE;
}

i64 getsizespec(const u8 *string) {
    u8 tmp[MAXTEMPSIZE];
    strcpy((char *)tmp, (char *)string);
    toUpper((u8 *)tmp);
    for (i64 i = 0; i < sizeof(size_specs) / sizeof(size_specs[0]); i++) {
        if (size_specs[i] == nullptr) continue;
        if (strcmp((char *)tmp, size_specs[i]) == 0) {
            return i;
        }
    }
    return SS_NONE;
}