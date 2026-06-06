#include "asmlib.h"
#include "../parser/ast.h"

#include <stdio.h>
#include <stdlib.h>
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
    "FR", // 11
    "CR0", // 12
    "CR1", // 13
    "NAN", // 14
    "NAN" // 15
};

char *size_specs[] = {
    [SS_BYTE] = "BYTE",
    [SS_WORD] = "WORD"
};

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

bool ismnemonic(u8 *string) {
    InstructionSpec spec = get_spec((char *)string);
    if (spec.operand_pattern.operand_count == -1) return false;
    return true;
}

bool isregister(const u8 *string) {
    u8 tmp[MAXTEMPSIZE];
    strcpy((char *)tmp, (char *)string);
    toUpper(tmp);
    for (int i = 0; i < sizeof(registers) / sizeof(registers[0]); i++) {
        if (strcmp((char *)tmp, registers[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool isSPR(registers_t reg) {
    if (reg == NONE) return false;
    return reg >= PC ? true : false;
}

bool issizespec(const u8 *string) {
    u8 tmp[MAXTEMPSIZE];
    strcpy((char *)tmp, (char *)string);
    toUpper(tmp);
    for (int i = 0; i < sizeof(size_specs) / sizeof(size_specs[0]); i++) {
        if (size_specs[i] == nullptr) continue;
        if (strcmp((char *)tmp, size_specs[i]) == 0) {
            return true;
        }
    }
    return false;
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

char *read_assembly(const char *file) {
    FILE *f = fopen(file, "r");

    fseek(f, 0, SEEK_END);
    const u64 len = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *code = malloc(len+10);
    memset(code, 0, len+10);

    fread(code, 1, len, f);

    fclose(f);

    return code;
}