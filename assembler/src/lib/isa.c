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
            {3, {REGISTER, SYMBOL, REGISTER}},
            {3, {REGISTER, REGISTER, SYMBOL}},
            {3, {REGISTER, SYMBOL, SYMBOL}},
            {3, {REGISTER, REGISTER, REG_INDIRECT}},
            {3, {REGISTER, REG_INDIRECT, REGISTER}},
            {3, {REGISTER, REG_INDIRECT, REG_INDIRECT}},
            {3, {REGISTER, SYMBOL, REG_INDIRECT}},
            {3, {REGISTER, REG_INDIRECT, SYMBOL}},
            {3, {REG_INDIRECT, REGISTER, REGISTER}},
            {3, {REG_INDIRECT, SYMBOL, REGISTER}},
            {3, {REG_INDIRECT, REGISTER, SYMBOL}},
            {3, {REG_INDIRECT, SYMBOL, SYMBOL}},
        },
        25
    },
    {
        "SUB",
        0,
        0x1,
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
            {3, {REGISTER, SYMBOL, REGISTER}},
            {3, {REGISTER, REGISTER, SYMBOL}},
            {3, {REGISTER, SYMBOL, SYMBOL}},
            {3, {REGISTER, REGISTER, REG_INDIRECT}},
            {3, {REGISTER, REG_INDIRECT, REGISTER}},
            {3, {REGISTER, REG_INDIRECT, REG_INDIRECT}},
            {3, {REGISTER, SYMBOL, REG_INDIRECT}},
            {3, {REGISTER, REG_INDIRECT, SYMBOL}},
            {3, {REG_INDIRECT, REGISTER, REGISTER}},
            {3, {REG_INDIRECT, SYMBOL, REGISTER}},
            {3, {REG_INDIRECT, REGISTER, SYMBOL}},
            {3, {REG_INDIRECT, SYMBOL, SYMBOL}},
        },
        25
    },
    {
        "AND",
        0,
        0x2,
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
            {3, {REGISTER, SYMBOL, REGISTER}},
            {3, {REGISTER, REGISTER, SYMBOL}},
            {3, {REGISTER, SYMBOL, SYMBOL}},
            {3, {REGISTER, REGISTER, REG_INDIRECT}},
            {3, {REGISTER, REG_INDIRECT, REGISTER}},
            {3, {REGISTER, REG_INDIRECT, REG_INDIRECT}},
            {3, {REGISTER, SYMBOL, REG_INDIRECT}},
            {3, {REGISTER, REG_INDIRECT, SYMBOL}},
            {3, {REG_INDIRECT, REGISTER, REGISTER}},
            {3, {REG_INDIRECT, SYMBOL, REGISTER}},
            {3, {REG_INDIRECT, REGISTER, SYMBOL}},
            {3, {REG_INDIRECT, SYMBOL, SYMBOL}},
        },
        25
    },
    {
        "OR",
        0,
        0x3,
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
            {3, {REGISTER, SYMBOL, REGISTER}},
            {3, {REGISTER, REGISTER, SYMBOL}},
            {3, {REGISTER, SYMBOL, SYMBOL}},
            {3, {REGISTER, REGISTER, REG_INDIRECT}},
            {3, {REGISTER, REG_INDIRECT, REGISTER}},
            {3, {REGISTER, REG_INDIRECT, REG_INDIRECT}},
            {3, {REGISTER, SYMBOL, REG_INDIRECT}},
            {3, {REGISTER, REG_INDIRECT, SYMBOL}},
            {3, {REG_INDIRECT, REGISTER, REGISTER}},
            {3, {REG_INDIRECT, SYMBOL, REGISTER}},
            {3, {REG_INDIRECT, REGISTER, SYMBOL}},
            {3, {REG_INDIRECT, SYMBOL, SYMBOL}},
        },
        25
    },
    {
        "XOR",
        0,
        0x4,
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
            {3, {REGISTER, SYMBOL, REGISTER}},
            {3, {REGISTER, REGISTER, SYMBOL}},
            {3, {REGISTER, SYMBOL, SYMBOL}},
            {3, {REGISTER, REGISTER, REG_INDIRECT}},
            {3, {REGISTER, REG_INDIRECT, REGISTER}},
            {3, {REGISTER, REG_INDIRECT, REG_INDIRECT}},
            {3, {REGISTER, SYMBOL, REG_INDIRECT}},
            {3, {REGISTER, REG_INDIRECT, SYMBOL}},
            {3, {REG_INDIRECT, REGISTER, REGISTER}},
            {3, {REG_INDIRECT, SYMBOL, REGISTER}},
            {3, {REG_INDIRECT, REGISTER, SYMBOL}},
            {3, {REG_INDIRECT, SYMBOL, SYMBOL}},
        },
        25
    },
    {
        "NOT",
        0,
        0x5,
        {
            {2, {REGISTER, REGISTER}},
            {2, {REGISTER, IMMEDIATE}},
            {2, {REGISTER, REG_INDIRECT}},
            {2, {REGISTER, SYMBOL}},
            {2, {REG_INDIRECT, REGISTER}},
            {2, {REG_INDIRECT, IMMEDIATE}},
        },
        6
    },

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// class 1: data movement
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    {
        "MOV",
        1,
        0x0,
        {
            {2, {REGISTER, REGISTER}},
            {2, {REGISTER, IMMEDIATE}},
            {2, {REGISTER, REG_INDIRECT}},
            {2, {REGISTER, SYMBOL}},
            {2, {REG_INDIRECT, REGISTER}},
            {2, {REG_INDIRECT, IMMEDIATE}},
        },
        6
    },
    {
        "MOVSR", // dest is GPR and source is a usage of an SPR
        1,
        0x1,
        {
            {2, {REGISTER, REGISTER}},
            {2, {REG_INDIRECT, REGISTER}},
            {2, {REGISTER, REG_INDIRECT}},
            {2, {REG_INDIRECT, REG_INDIRECT}},
        },
        4
    },
    {
        "MOVRS", // dest is usage of SPR and source is SPR/immediate/memory...
        1,
        0x2,
        {
            {2, {REGISTER, REGISTER}},
            {2, {REGISTER, REG_INDIRECT}},
            {2, {REGISTER, IMMEDIATE}},
            {2, {REGISTER, SYMBOL}},
            {2, {REG_INDIRECT, REGISTER}},
            {2, {REG_INDIRECT, IMMEDIATE}},
            {2, {REG_INDIRECT, SYMBOL}},
        },
        7
    },
    {
        "PUSH",
        1,
        0x3,
        {
            {1, {REGISTER}},
            {1, {IMMEDIATE}},
            {1, {REG_INDIRECT}},
            {1, {SYMBOL}},
        },
        4
    },
    {
        "POP",
        1,
        0x4,
        {
            {1, {REGISTER}},
            {1, {REG_INDIRECT}},
        },
        2
    },
    {
        "LEA",
        1,
        0x5,
        {
            {2, {REGISTER, REG_INDIRECT}},
        },
        1
    },
    {
        "MOVS",
        1,
        0x6,
        {
            {2, {REGISTER, REGISTER}},
        }
    },
    {
        "PUSHS",
        1,
        0x7,
        {
            {1, {REGISTER}},
            {1, {REG_INDIRECT}},
        }
    },
    {
        "POPS",
        1,
        0x8,
        {
            {1, {REGISTER}},
            {1, {REG_INDIRECT}},
        }
    },

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// class 2: control flow
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    {
        "JMP",
        2,
        0x0,
        {
            {1, {REGISTER}},
            {1, {IMMEDIATE}},
            {1, {REG_INDIRECT}},
            {1, {SYMBOL}}
        },
        4
    },

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// class 3: system instructions
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    {
        "NOP",
        3,
        0x0,
        {
            {0, {0}}
        },
        1
    },
    {
        "HLT",
        3,
        0x1,
        {
            {0, {0}}
        },
        1
    }
};

char *cond_jump[] = {
    "JZ", "JE",     // ZF = 1
    "JNZ", "JNE",   // ZF = 0
    "JC",           // CF = 1
    "JNC",          // CF = 0
    "JO",           // OF = 1
    "JNO",          // OF = 0
    "JS",           // SF = 1
    "JNS",          // SF = 0
    "JA",           // ZF = 0 and CF = 0
    "JAE",          // CF = 0
    "JB",           // CD = 1
    "JBE",          // ZF = 1 or CF = 1
    "JG",           // ZF = 0 and SF = OF
    "JGE",          // SF = OF
    "JL",           // SF != OF
    "JLE",          // ZF = 1 or sf != OF
};

u64 cond_jump_size = sizeof(cond_jump) / sizeof(cond_jump[0]);

u64 isa_size = sizeof(ISA) / sizeof(ISA[0]);

i64 is_cond_jump(const char *mnemonic) {
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
    {
        {1, {DISPLACEMENT}},
        {1, {SYMBOL}},
    },
    2
};

InstructionSpec get_spec(const char *mnemonic) {
    char buff[MAXTEMPSIZE];
    strcpy(buff, mnemonic);
    toUpper((u8 *)buff);

    i64 cond_jump_idx;
    if ((cond_jump_idx = is_cond_jump(buff)) >= 0) {
        InstructionSpec ret = {0};
        memcpy(ret.mnemonic, &cond_jump_template, sizeof(InstructionSpec));
        ret.mnemonic = strdup(cond_jump[cond_jump_idx]);

        if (cond_jump_idx == 0 || cond_jump_idx == 1) {
            ret.opcode = 1;
        } else if (cond_jump_idx == 2 || cond_jump_idx == 3) {
            ret.opcode = 2;
        } else {
            ret.opcode = cond_jump_idx - 1;
        }

        return ret;
    }

    for (u64 i = 0; i < isa_size; i++) {
        if (strcmp(ISA[i].mnemonic, buff) == 0) {
            return ISA[i];
        }
    }
    InstructionSpec ret = {0};
    ret.signature_count = -1;
    return ret;
}