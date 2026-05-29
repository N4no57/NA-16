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
        "MOVSR",
        1,
        0x1,
        {
            {2, {REGISTER, REGISTER}},
            {2, {REG_INDIRECT, REGISTER}},
        },
        2
    },
    {
        "MOVRS",
        1,
        0x2,
        {
            {2, {REGISTER, REGISTER}},
            {2, {REGISTER, REG_INDIRECT}},
            {2, {REGISTER, IMMEDIATE}},
            {2, {REGISTER, SYMBOL}},
        },
        4
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

u64 isa_size = sizeof(ISA) / sizeof(ISA[0]);

InstructionSpec get_spec(const char *mnemonic) {
    char buff[MAXTEMPSIZE];
    strcpy(buff, mnemonic);
    toUpper((u8 *)buff);

    for (u64 i = 0; i < isa_size; i++) {
        if (strcmp(ISA[i].mnemonic, buff) == 0) {
            return ISA[i];
        }
    }
    InstructionSpec ret = {0};
    ret.signature_count = -1;
    return ret;
}