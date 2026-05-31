#include "codegen.h"
#include "../lib/error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

i16 get_register_encoding(registers_t reg) {
    if (reg <= R7) {
        return reg;
    }

    if (reg >= PC) {
        return reg - PC;
    }

    return -1; // idfk what to do with the rest of the registers
}

void push_bytes(bytes *code, const u8 *buff, const u8 *buff_size) {
    if (code->count + *buff_size >= code->size) {
        code->size += *buff_size;
        code->size *= 2;
        u8 *tmp = realloc(code->data, code->size * sizeof(u8));
        if (!tmp) {
            free(code->data);
            exit(1);
        }
        code->data = tmp;
    }

    for (u8 i = 0; i < *buff_size; i++) {
        code->data[code->count++] = buff[i];
    }
}

void visit_NodeOperand(const NodeOperand *operand, bool use_16bits, u8 *buff, u8 *idx) {
    if (operand->kind == REGISTER || operand->kind == REG_INDIRECT) {
        i16 status = get_register_encoding(operand->reg);
        if (status < 0) {
            error(operand->pos, "Invalid register");
        }
        buff[(*idx)++] = status;
    } else if (operand->kind == IMMEDIATE || operand->kind == DISPLACEMENT) {
        if (!use_16bits) {
            buff[(*idx)++] = operand->immediate & 0xFF;
        } else {
            buff[(*idx)++] = operand->immediate & 0xFF;
            buff[(*idx)++] = (operand->immediate & 0xFF00) >> 8;
        }
    } else {
        error(operand->pos, "Invalid operand type");
    }
}

typedef struct {
    u8 size[3];
    u8 offset[3];
} OperandLayout;

OperandLayout layout_for(const InstructionSignature *sig, bool use_16bits) {
    OperandLayout l = {0};

    for (i32 i = 0; i < sig->operand_count; i++) {
        if (sig->kinds[i] == REGISTER || sig->kinds[i] == REG_INDIRECT) {
            l.size[i] = 1;
        } else if (sig->kinds[i] == IMMEDIATE || sig->kinds[i] == DISPLACEMENT) {
            l.size[i] = use_16bits ? 2 : 1;
        }
    }

    l.offset[0] = 0;
    for (i32 i = 1; i < sig->operand_count; i++) {
        l.offset[i] = l.offset[i-1] + l.size[i-1];
    }

    return l;
}

u16 pack_registers(const InstructionSignature *sig, u8 **op, u8 op_count) {
    u16 reg_pack = 0;

    for (i32 i = 0; i < op_count; i++) {
        if (sig->kinds[i] == REGISTER || sig->kinds[i] == REG_INDIRECT) {
            reg_pack |= (*op[i] & 0x7) << (6 - 3 * i);
        }
    }

    return reg_pack;
}

void fold(const char *mnemonic, const InstructionSignature *sig, bool use_16bits, u8 inst_slot, u8 *buff, u8 *idx) {
    char mnemonic_buff[MAXTEMPSIZE];
    strcpy(mnemonic_buff, mnemonic);
    toUpper((u8 *)mnemonic_buff);

    if (strcmp(mnemonic_buff, "HLT") == 0) {
        return;
    }

    if (strcmp(mnemonic_buff, "NOP") == 0) {
        return;
    }

    if (strcmp(mnemonic_buff, "RET") == 0) {
        return;
    }

    if (sig->operand_count == 0) {
        fatal((Position){nullptr, nullptr, 0, 0, 0}, "AAAAAAAAAAAAAAAAAAAAAAAAAA");
    }

    OperandLayout l = layout_for(sig, use_16bits);
    u8 *base = &buff[inst_slot+2];

    u8 *op[3] = {0};

    for (i32 i = 0; i < sig->operand_count; i++) {
        op[i] = base + l.offset[i];
    }

    u16 reg_pack = pack_registers(sig, op, sig->operand_count);
    buff[inst_slot]     |= (reg_pack & 0xFF00) >> 8;
    buff[inst_slot+1]   |= reg_pack & 0xFF;

    u8 reg_count_behind[3] = {0};

    for (i32 i = 1; i < sig->operand_count; i++) {
        reg_count_behind[i] = reg_count_behind[i-1];
        if (sig->kinds[i-1] == REGISTER || sig->kinds[i-1] == REG_INDIRECT) {
            reg_count_behind[i]++;
        }
    }

    for (i32 i = 0; i < sig->operand_count; i++) {
        if (reg_count_behind[i] > 0) {
            if (sig->kinds[i] == IMMEDIATE || sig->kinds[i] == DISPLACEMENT) {
                memcpy(op[i]-reg_count_behind[i], op[i], l.size[i]);
            }
        }
    }

    u8 removal = reg_count_behind[sig->operand_count-1];
    if (sig->kinds[sig->operand_count-1] == REGISTER || sig->kinds[sig->operand_count-1] == REG_INDIRECT) {
        removal++;
    }

    *idx -= removal;
}

void visit_NodeInstruction(const NodeInstruction *node, bytes *code) {
    u8 buff[MAXTEMPSIZE] = {0};
    u8 buff_idx = 0;

    InstructionSpec info = get_spec((char *)node->mnemonic);

    u64 sig_id = 0;
    InstructionSignature *sig = &info.signatures[sig_id];
    for (sig_id = 0; sig_id < info.signature_count; sig_id++) {
        if (match_signature(node, sig)) break;
        sig = &info.signatures[sig_id+1];
    }

    if (sig_id > 0 && !is_cond_jump(node->mnemonic)) {
        u16 MEX_prefix = 0;
        if (sig->operand_count == 3) {
            MEX_prefix = GEN_MEX(sig->kinds[0], sig->kinds[1], sig->kinds[2]);
        } else if (sig->operand_count == 2) {
            MEX_prefix = GEN_MEX(sig->kinds[0], sig->kinds[1], 0);
        } else if (sig->operand_count == 1) {
            MEX_prefix = GEN_MEX(sig->kinds[0], 0, 0);
        }

        if (MEX_prefix == 0) {
            fatal(node->pos, "MEX prefix error\n");
        }

        buff[buff_idx++] = (MEX_prefix & 0xFF00) >> 8;
        buff[buff_idx++] = MEX_prefix & 0xFF;
    }

    bool use_16bits = require_16_bits(node, sig);

    if (use_16bits) {
        buff[buff_idx++] = GEN_AEX;
    }

    if (info.opcode > 0xF) {
        buff[buff_idx++] = GEN_ESCAPE_BYTE;
    }

    u8 inst_slot = buff_idx;
    buff_idx += 2; // reserve space for the base instruction encoding

    for (u8 i = 0; i < node->operand_count; i++) {
        visit_NodeOperand(&node->operands[i], use_16bits, buff, &buff_idx);
    }

    buff[inst_slot] |= (info.class & 0x7) << 5 | (info.opcode & 0xF) << 1;

    fold(node->mnemonic, sig, use_16bits, inst_slot, buff, &buff_idx);

    push_bytes(code, buff, &buff_idx);
}

void visit_NodeStatement(const NodeStatement *node, bytes *code) {
    if (node->kind == ST_INSTRUCTION) {
        visit_NodeInstruction(&node->instruction, code);
    }
}

void generate_code(NodeProgram *ast, bytes *code) {
    if (!ast) return;

    symbol_pass(ast);

    if (error_count > 0 ) {
        if (error_count == 1) printf("Found 1 error:\n");
        else printf("Found %d errors:\n", error_count);

        exit(EXIT_FAILURE);
    }

    for (u64 i = 0; i < ast->count; i++) {
        visit_NodeStatement(&ast->statements[i], code);
    }
}
