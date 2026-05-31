#include <stdio.h>
#include <string.h>

#include "debugger.h"

#include <stdlib.h>

#include "../cpu.h"
#include "../instructions/inst.h"

u32 breakpoints[MAX_BREAKPOINTS];

char *get_register(const u16 reg) {
    switch (reg) {
        case 0x0: // R0
            return "R0";
        case 0x1: // R1
            return "R1";
        case 0x2: // R2
            return "R2";
        case 0x3: // R3
            return "R3";
        case 0x4:
            return "R4";
        case 0x5:
            return "R5";
        case 0x6:
            return "R6";
        case 0x7:
            return "R7";
        case 0x1 << 6:
            return "PC";
        case 0x2 << 6:
            return "SP";
        case 0x3 << 6:
            return "BP";
        default:
            return nullptr;
    }
}

void print_operands(const CPU *cpu, const Instruction *inst, const InstructionDef *def) {
    char format[] = "%s, ";

    if (is_cond_jump(inst)) {
        format[2] = ' ';
        format[3] = '\0';
        i16 value = inst->ops[0].displacement;
        char sign[] = "+\0\0";
        if (value < 0) sign[0] = '-';
        if (strlen(def->name) < 3) {
            sign[1] = sign[0];
            sign[0] = '\t';
            sign[2] = '\0';
        }
        printf("%s%d", sign, value);
        return;
    }

    for (u64 i = 0; i < inst->op_count; i++) {
        if (i == inst->op_count - 1) {
            format[2] = '\0';
        }
        switch (inst->ops[i].mode) {
            case OP_REG: // register
                char *reg = get_register(inst->ops[i].reg);
                if (nullptr == reg) break;
                printf(format, reg);
                break;
            case OP_IMM: // immediate
                format[1] = 'd';
                printf(format, inst->ops[i].immediate);
                format[1] = 's';
                break;
            case OP_REG_IND:
                char *reg_ind = get_register(inst->ops[i].reg);
                if (nullptr == reg_ind) break;
                char thingy[100];
                snprintf(thingy, 100, "%c%s%c", '[', format, ']');
                if (format[2] == ',') {
                    thingy[3] = ']';
                    thingy[4] = ',';
                    thingy[5] = ' ';
                    thingy[6] = '\0';
                }
                printf(thingy, reg_ind);
                break;
            default:
                break;
        }
    }
}

void print_instruction(const CPU *cpu, const Instruction *inst, const InstructionDef *def) {
    printf("%llu", cpu->PC - inst->size);
    printf("\t\t%s", def->name);
    if (inst->ops[0].size == 1) printf(" byte\t");
    else if (inst->ops[0].size == 2) printf(" word\t");
    print_operands(cpu, inst, def);

    printf("\n");
}

void commands(bool *halt) {
    while (true) {
        char command = (char)getc(stdin);
        if (command == '\n') continue;

        switch (command) {
            case 'c':
                *halt = false;
                return;
            case 'b':
                i32 address = 0;
                scanf("%d", &address);
                if (address > 0xFFFF || address < 0) {
                    break;
                }
                add_breakpoint(address);
                printf("Breakpoint set at %d\n", address);
                break;
            case 's':
                return;
            default:
                printf("Not a valid command\n");
                break;
        }
    }
}

bool add_breakpoint(u16 address) {
    u16 i = 0;
    while (i < MAX_BREAKPOINTS) {
        if ((breakpoints[i] & 0xFFFF) == address) {
            return true;
        }

        if ((breakpoints[i] >> 16 & 0x1) == false) {
            break;
        }
        i++;
    }

    if (i == MAX_BREAKPOINTS) {
        return false;
    }

    breakpoints[i] = address;
    breakpoints[i] |= 0x10000;

    return true;
}

bool remove_breakpoint(u16 address) {
    u16 i = 0;
    while (i < MAX_BREAKPOINTS) {
        if ((breakpoints[i] & 0xFFFF) == address) {
            break;
        }

        i++;
    }

    if (i == MAX_BREAKPOINTS) {
        return false;
    }

    breakpoints[i] = 0;
    return true;
}

bool check_breakpoint(CPU *cpu) {
    for (u64 i = 0; i < MAX_BREAKPOINTS; i++) {
        if ((breakpoints[i] & 0xFFFF) == cpu->PC) {
            return true;
        }
    }

    return false;
}