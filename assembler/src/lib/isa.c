#include "asmlib.h"

#include <string.h>

InstructionSpec ISA[] = {
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// class 0: ALU operations
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    {
        "ADD",
        0,
        0x0,
        {3,
            {CLASS_DEST, CLASS_SOURCE, CLASS_SOURCE}
        }
    },
    {
        "SUB",
        0,
        0x1,
        {3,
            {CLASS_DEST, CLASS_SOURCE, CLASS_SOURCE}
        }
    },
    {
        "AND",
        0,
        0x2,
        {3,
            {CLASS_DEST, CLASS_SOURCE, CLASS_SOURCE}
        }
    },
    {
        "OR",
        0,
        0x3,
        {3,
            {CLASS_DEST, CLASS_SOURCE, CLASS_SOURCE}
        }
    },
    {
        "XOR",
        0,
        0x4,
        {3,
            {CLASS_DEST, CLASS_SOURCE, CLASS_SOURCE}
        }
    },
    {
        "NOT",
        0,
        0x5,
        {2,
            {CLASS_DEST, CLASS_SOURCE}
        }
    },
    {
        "CMP",
        0,
        0x6,
        {2,
            {CLASS_DEST, CLASS_SOURCE}
        }
    },
    {
        "TEST",
        0,
        0x7,
        {2,
            {CLASS_DEST, CLASS_SOURCE}
        }
    },

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// class 1: data movement
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    {
        "MOV",
        1,
        0x0,
        {2,
            {CLASS_DEST, CLASS_SOURCE}
        }
    },
    {
        "MOVSR", // dest is GPR and source is a usage of an SPR
        1,
        0x1,
        {2,
            {CLASS_DEST, CLASS_REGISTER_ONLY}
        }
    },
    {
        "MOVRS", // dest is usage of SPR and source is GPR/immediate/memory...
        1,
        0x2,
        {2,
            {CLASS_DEST, CLASS_SOURCE}
        }
    },
    {
        "PUSH",
        1,
        0x3,
        {2,
            {CLASS_DEST, CLASS_SOURCE}
        }
    },
    {
        "POP",
        1,
        0x4,
        {1,
            {CLASS_DEST},
        }
    },
    {
        "LEA",
        1,
        0x5,
        {2,
            {CLASS_DEST, CLASS_DEST},
        }
    },
    {
        "MOVS",
        1,
        0x6,
        {2,
            {CLASS_REGISTER_ONLY, CLASS_REGISTER_ONLY},
        }
    },
    {
        "PUSHS",
        1,
        0x7,
        {1,
            {CLASS_DEST},
        }
    },
    {
        "POPS",
        1,
        0x8,
        {1,
            {CLASS_REGISTER_ONLY},
        }
    },

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// class 2: control flow
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    {
        "JMP",
        2,
        0x0,
        {1,
            {CLASS_SOURCE}
        }
    },
    {
        "RET",
        2,
        0xF,
        {0}
    },
    {
        "CALL",
        2,
        0x10,
        {1,
            {CLASS_SOURCE}
        }
    },

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// class 3: system instructions
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    {
        "NOP",
        3,
        0x0,
        {0}
    },
    {
        "HLT",
        3,
        0x1,
        {0}
    }
};

char *cond_jump[] = {
    "JZ", "JE",     // ZF = 1
    "JNZ", "JNE",   // ZF = 0
    "JC", "JB",     // CF = 1
    "JNC", "JAE",   // CF = 0
    "JO",           // OF = 1
    "JNO",          // OF = 0
    "JS",           // SF = 1
    "JNS",          // SF = 0
    "JA",           // ZF = 0 and CF = 0
    "JBE",          // ZF = 1 or CF = 1
    "JG",           // ZF = 0 and SF = OF
    "JGE",          // SF = OF
    "JL",           // SF != OF
    "JLE",          // ZF = 1 or sf != OF
};

u64 cond_jump_size = sizeof(cond_jump) / sizeof(cond_jump[0]);

u64 isa_size = sizeof(ISA) / sizeof(ISA[0]);

bool is_cond_jump(const char *mnemonic) {
    char buff[MAXTEMPSIZE];
    strcpy(buff, mnemonic);
    toUpper((u8 *)buff);

    for (u64 i = 0; i < cond_jump_size; i++) {
        if (strcmp(cond_jump[i], buff) == 0) return true;
    }
    return false;
}

i64 get_cond_jump(const char *mnemonic) {
    char buff[MAXTEMPSIZE];
    strcpy(buff, mnemonic);
    toUpper((u8 *)buff);

    for (u64 i = 0; i < cond_jump_size; i++) {
        if (strcmp(cond_jump[i], buff) == 0) return (i64)i;
    }
    return -1;
}

InstructionSpec cond_jump_template = {
    nullptr,
    2,
    0x0,
    {1,
        {CLASS_DISP_OR_SYM}
    }
};

InstructionSpec get_spec(const char *mnemonic) {
    char buff[MAXTEMPSIZE];
    strcpy(buff, mnemonic);
    toUpper((u8 *)buff);

    if (is_cond_jump(buff)) {
        i64 cond_jump_idx = get_cond_jump(buff);
        InstructionSpec ret = {0};
        memcpy(&ret, &cond_jump_template, sizeof(InstructionSpec));
        ret.mnemonic = strdup(cond_jump[cond_jump_idx]);

        switch (cond_jump_idx) {
            case 0:
            case 1:
                ret.opcode = 1;
                break;
            case 2:
            case 3:
                ret.opcode = 2;
                break;
            case 4:
            case 5:
                ret.opcode = 3;
                break;
            case 6:
            case 7:
                ret.opcode = 4;
                break;
            default:
                ret.opcode = cond_jump_idx - 2;
        }

        return ret;
    }

    for (u64 i = 0; i < isa_size; i++) {
        if (strcmp(ISA[i].mnemonic, buff) == 0) {
            return ISA[i];
        }
    }
    InstructionSpec ret = {0};
    ret.operand_pattern.operand_count = -1;
    return ret;
}

bool operand_matches_class(operand_types type, OperandClass cls, bool is_cond_jump) {
    if (is_cond_jump) {
        if (cls != CLASS_DISP_OR_SYM) return false;
        return type == DISPLACEMENT || type == SYMBOL;
    }

    switch (cls) {
        case CLASS_DEST:
            return type == REGISTER     ||
                type == REG_INDIRECT    ||
                type == REG_IND_DISP    ||
                type == ABSOLUTE        ||
                type == SIB             ||
                type == SIB_DISP;
        case CLASS_SOURCE:
            return type == REGISTER     ||
                type == REG_INDIRECT    ||
                type == REG_IND_DISP    ||
                type == ABSOLUTE        ||
                type == SIB             ||
                type == SIB_DISP        ||
                type == IMMEDIATE       ||
                type == SYMBOL;
        default:
            return false;
    }
}