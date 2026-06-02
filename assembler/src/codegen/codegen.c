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

void push_bytes(bytes *code, const u8 *buff, const u64 buff_size) {
    while (code->count + buff_size >= code->size) {
        code->size += buff_size;
        code->size *= 2;
        u8 *tmp = realloc(code->data, code->size * sizeof(u8));
        if (!tmp) {
            free(code->data);
            exit(1);
        }
        code->data = tmp;
    }


    memcpy(&code->data[code->count], buff, buff_size);

    code->count += buff_size;
}

void visit_NodeOperand(const NodeOperand *operand, bool use_16bits, u8 *buff, u8 *idx) {
    if (operand->kind == REGISTER || operand->kind == REG_INDIRECT) {
        i16 status = get_register_encoding(operand->reg);
        if (status < 0) {
            error(operand->pos, "Invalid register");
        }
        buff[(*idx)++] = status;
    } else if (operand->kind == IMMEDIATE) {
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

    if (is_cond_jump(node->mnemonic)) {
        // handle this crap
        bool use_16bits = wont_fit_s8(node->operands[0].immediate);
        if (use_16bits) buff[buff_idx++] = GEN_AEX;

        buff[buff_idx] |= (info.class & 0x7) << 5 | (info.opcode & 0xF) << 1;
        buff_idx += 2;

        const NodeOperand *operand = &node->operands[0];
        if (!use_16bits) {
            buff[buff_idx++] = operand->immediate & 0xFF;
        } else {
            buff[buff_idx++] = operand->immediate & 0xFF;
            buff[buff_idx++] = (operand->immediate & 0xFF00) >> 8;
        }

        push_bytes(code, buff, buff_idx);
        return;
    }

    u64 sig_id = 0;
    InstructionSignature *sig = &info.signatures[sig_id];
    for (sig_id = 0; sig_id < info.signature_count; sig_id++) {
        if (match_signature(node, sig)) break;
        sig = &info.signatures[sig_id+1];
    }

    if (sig_id > 0) {
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

    push_bytes(code, buff, buff_idx);
}

void emit_define(const NodeDirective *node, bytes *code, u64 size) {
    u64 tok_idx = 0;
    Token *tok = &node->args.tokens[tok_idx];
    while (tok_idx < node->args.count) {
        if (tok->type != TT_IMMEDIATE) {
            error(tok->pos, "invalid argument for \"%s\" directive", node->name);
        }

        push_bytes(code, tok->value, size);

        tok = &node->args.tokens[++tok_idx];
    }
}

void visit_NodeDirective(const NodeDirective *node, bytes *code) {
    u64 tok_idx = 0;
    Token *tok = &node->args.tokens[tok_idx];

    if (strcmp(node->name, ".db") == 0) {
        emit_define(node, code, 1);
        return;
    }

    if (strcmp(node->name, ".dw") == 0) {
        emit_define(node, code, 2);
        return;
    }

    if (strcmp(node->name, ".dd") == 0) {
        emit_define(node, code, 4);
        return;
    }

    if (strcmp(node->name, ".dq") == 0) {
        emit_define(node, code, 8);
        return;
    }

    if (strcmp(node->name, ".ascii") == 0) {
        while (tok_idx < node->args.count) {
            if (tok->type != TT_STRING) {
                error(tok->pos, "invalid argument for \"%s\" directive", node->name);
            }

            String *s = tok->value;

            push_bytes(code, (u8 *)s->str, s->size);

            tok = &node->args.tokens[++tok_idx];
        }

        return;
    }

    if (strcmp(node->name, ".asciz") == 0) {
        while (tok_idx < node->args.count) {
            if (tok->type != TT_STRING) {
                error(tok->pos, "invalid argument for \"%s\" directive", node->name);
            }

            String *s = tok->value;
            char null_terminator = '\0';

            push_bytes(code, (u8 *)s->str, s->size);
            push_bytes(code, (u8 *)&null_terminator, 1);

            tok = &node->args.tokens[++tok_idx];
        }

        return;
    }

    error(node->pos, "invalid directive");
}

void visit_NodeStatement(const NodeStatement *node, bytes *code) {
    if (node->kind == ST_INSTRUCTION) {
        visit_NodeInstruction(&node->instruction, code);
    } else if (node->kind == ST_DIRECTIVE) {
        visit_NodeDirective(&node->directive, code);
    } else if (node->kind == ST_SYMBOL) {
        return;
    } else {
        error((Position){0}, "Excuse me what the actual fuck are you doing in my house?");
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
