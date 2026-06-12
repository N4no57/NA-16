#include "codegen.h"
#include "../lib/error.h"
#include "../obj_file_writer/sections.h"

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

void push_bytes(Section *section, const u8 *buff, const u64 buff_size) {
    while (section->count + buff_size >= section->size) {
        section->size += buff_size;
        section->size *= 2;
        u8 *tmp = realloc(section->data, section->size * sizeof(u8));
        if (!tmp) {
            free(section->data);
            exit(1);
        }
        section->data = tmp;
    }


    memcpy(&section->data[section->count], buff, buff_size);

    section->count += buff_size;
}

void encode_number(i64 number, u8 *buff, u8 *idx, u8 size) {
    for (u8 i = 0; i < size; i++) {
        buff[*idx] = number >> (8 * i) & 0xFF;
        *idx += 1;
    }
}

void visit_NodeOperand(const NodeOperand *operand, bool use_16bits, u8 *buff, u8 *idx) {
    if (operand->kind == REGISTER || operand->kind == REG_INDIRECT) {
        i16 status = get_register_encoding(operand->reg);
        if (status < 0) {
            error(operand->pos, "Invalid register");
        }
        buff[(*idx)++] = status;
    } else if (operand->kind == IMMEDIATE || operand->kind == ABSOLUTE) {
        encode_number(operand->immediate, buff, idx, use_16bits + 1);
    } else if (operand->kind == REG_IND_DISP) {
        i16 status = get_register_encoding(operand->reg);
        if (status < 0) {
            error(operand->pos, "Invalid register");
        }
        buff[(*idx)++] = status;

        encode_number(operand->immediate, buff, idx, use_16bits + 1);
    } else if (operand->kind == SIB) {
        i16 status = get_register_encoding(operand->reg);
        if (status < 0)error(operand->pos, "Invalid register");
        buff[(*idx)++] = status;

        status = get_register_encoding(operand->idx_reg);
        if (status < 0) error(operand->pos, "Invalid register");
        buff[(*idx)++] = status;

        buff[(*idx)++] = operand->scale;
    } else if (operand->kind == SIB_DISP) {
        i16 status = get_register_encoding(operand->reg);
        if (status < 0)error(operand->pos, "Invalid register");
        buff[(*idx)++] = status;

        status = get_register_encoding(operand->idx_reg);
        if (status < 0) error(operand->pos, "Invalid register");
        buff[(*idx)++] = status;

        buff[(*idx)++] = operand->scale;

        encode_number(operand->immediate, buff, idx, use_16bits + 1);
    } else {
        error(operand->pos, "Invalid operand type");
    }
}

typedef struct {
    u8 size[3];
    u8 offset[3];
} OperandLayout;

OperandLayout layout_for(const InstructionSpec *info, const NodeInstruction *node, bool use_16bits) {
    OperandLayout l = {0};
    for (i32 i = 0; i < info->operand_pattern.operand_count; i++) {
        if (node->operands[i].kind == REGISTER ||
            node->operands[i].kind == REG_INDIRECT) {
            l.size[i] = 1;
        } else if (node->operands[i].kind == IMMEDIATE ||
            node->operands[i].kind == DISPLACEMENT ||
            node->operands[i].kind == ABSOLUTE) {
            l.size[i] = use_16bits ? 2 : 1;
        } else if (node->operands[i].kind == REG_IND_DISP) {
            l.size[i] = 1;
            l.size[i] += use_16bits ? 2 : 1;
        } else if (node->operands[i].kind == SIB) {
            l.size[i] = 2;
        } else if (node->operands[i].kind == SIB_DISP) {
            l.size[i] = 3;
            l.size[i] += use_16bits ? 2 : 1;
        }
    }

    l.offset[0] = 0;
    for (i32 i = 1; i < info->operand_pattern.operand_count; i++) {
        l.offset[i] = l.offset[i-1] + l.size[i-1];
    }

    return l;
}

u16 pack_registers(const NodeInstruction *node, u8 **op, u8 op_count) {
    u16 reg_pack = 0;

    for (i32 i = 0; i < op_count; i++) {
        if (node->operands[i].kind == REGISTER || node->operands[i].kind == REG_INDIRECT ||
            node->operands[i].kind == REG_IND_DISP || node->operands[i].kind == SIB ||
            node->operands[i].kind == SIB_DISP) {
            reg_pack |= (*op[i] & 0x7) << (6 - 3 * i);
        }
    }

    return reg_pack;
}

void fold(const char *mnemonic, const InstructionSpec *info, const NodeInstruction *node, bool use_16bits, u8 inst_slot, u8 *buff, u8 *idx) {
    char mnemonic_buff[MAXTEMPSIZE];
    strcpy(mnemonic_buff, mnemonic);
    toUpper((u8 *)mnemonic_buff);

    if (info->class == 3) {
        return;
    }

    if (strcmp(mnemonic_buff, "RET") == 0) {
        return;
    }

    if (info->operand_pattern.operand_count == 0) {
        fatal((Position){nullptr, nullptr, 0, 0, 0}, "AAAAAAAAAAAAAAAAAAAAAAAAAA");
    }

    OperandLayout l = layout_for(info, node, use_16bits);
    u8 *base = &buff[inst_slot+2];

    u8 *op[3] = {0};

    for (i32 i = 0; i < info->operand_pattern.operand_count; i++) {
        op[i] = base + l.offset[i];
    }

    u16 reg_pack = pack_registers(node, op, info->operand_pattern.operand_count);
    buff[inst_slot]     |= (reg_pack & 0xFF00) >> 8;
    buff[inst_slot+1]   |= reg_pack & 0xFF;

    for (i32 i = 0; i < info->operand_pattern.operand_count; i++) {
        if (node->operands[i].kind == SIB || node->operands[i].kind == SIB_DISP) {
            u8 idx_reg = *(op[i]+1);
            u8 scale = *(op[i]+2);
            if (scale == 1) {
                scale = 0;
            } else if (scale == 2) {
                scale = 1;
            } else if (scale == 4) {
                scale = 2;
            } else if (scale == 8) {
                scale = 3;
            }
            u8 pack = (scale & 0x3) << 6;
            pack |= (idx_reg & 0x7) << 3;
            *op[i] = pack;
        }
    }

    u8 reg_count_behind[3] = {0};

    for (i32 i = 1; i < info->operand_pattern.operand_count; i++) {
        reg_count_behind[i] = reg_count_behind[i-1];
        if (node->operands[i-1].kind == REGISTER || node->operands[i-1].kind == REG_INDIRECT ||
            node->operands[i-1].kind == REG_IND_DISP) {
            reg_count_behind[i]++;
        } else if (node->operands[i-1].kind == SIB ||node->operands[i-1].kind == SIB_DISP) {
            reg_count_behind[i] += 2;
        }
    }

    if (node->operands[0].kind == REG_IND_DISP) {
        memmove(op[0], op[0]+1, l.size[0]-1);
    } else if (node->operands[0].kind == SIB_DISP) {
        memmove(op[0]+1, op[0]+3, l.size[0]-1);
    }

    for (i32 i = 0; i < info->operand_pattern.operand_count; i++) {
        if (reg_count_behind[i] > 0) {
            if (node->operands[i].kind == IMMEDIATE || node->operands[i].kind == DISPLACEMENT ||
                node->operands[i].kind == ABSOLUTE) {
                memmove(op[i]-reg_count_behind[i], op[i], l.size[i]);
            } else if (node->operands[i].kind == REG_IND_DISP) {
                memmove(op[i]-reg_count_behind[i], op[i]+1, l.size[i]-1);
            } else if (node->operands[i].kind == SIB) {
                memmove(op[i]-reg_count_behind[i], op[i], 1);
            } else if (node->operands[i].kind == SIB_DISP) {
                memmove(op[i]-reg_count_behind[i], op[i], 1);
                memmove(op[i]+1-reg_count_behind[i], op[i]+3, l.size[i]-3);
            }
        }
    }

    u8 removal = reg_count_behind[info->operand_pattern.operand_count-1];
    if (node->operands[info->operand_pattern.operand_count-1].kind == REGISTER
        || node->operands[info->operand_pattern.operand_count-1].kind == REG_INDIRECT
        || node->operands[info->operand_pattern.operand_count-1].kind == REG_IND_DISP) {
        removal++;
    } else if (node->operands[info->operand_pattern.operand_count-1].kind == SIB ||
        node->operands[info->operand_pattern.operand_count-1].kind == SIB_DISP) {
        removal += 2;
    }

    *idx -= removal;
}

void visit_NodeInstruction(const NodeInstruction *node, SectionTable *sections) {
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

        push_bytes(&sections->sections[sections->current], buff, buff_idx);
        return;
    }

    if (info.operand_pattern.operand_count != 0) {
        u16 MEX_prefix = 0;
        if (info.operand_pattern.operand_count == 3) {
            MEX_prefix = GEN_MEX(node->operands[0].kind, node->operands[1].kind, node->operands[2].kind);
        } else if (info.operand_pattern.operand_count == 2) {
            MEX_prefix = GEN_MEX(node->operands[0].kind, node->operands[1].kind, 0);
        } else if (info.operand_pattern.operand_count == 1) {
            MEX_prefix = GEN_MEX(node->operands[0].kind, 0, 0);
        }

        if (MEX_prefix == 0) {
            fatal(node->pos, "MEX prefix error\n");
        }

        if (MEX_prefix & 0xFFF) {
            buff[buff_idx++] = (MEX_prefix & 0xFF00) >> 8;
            buff[buff_idx++] = MEX_prefix & 0xFF;
        }
    }

    bool use_16bits = require_16_bits(node, info.operand_pattern.operand_count);

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

    fold(node->mnemonic, &info, node, use_16bits, inst_slot, buff, &buff_idx);

    push_bytes(&sections->sections[sections->current], buff, buff_idx);
}

void emit_define(const NodeDirective *node, Section *section, u64 size) {
    u64 tok_idx = 0;
    Token *tok = &node->args.tokens[tok_idx];
    while (tok_idx < node->args.count) {
        if (tok->type != TT_IMMEDIATE) {
            error(tok->pos, "invalid argument for \"%s\" directive", node->name);
        }

        push_bytes(section, tok->value, size);

        tok = &node->args.tokens[++tok_idx];
    }
}

void visit_NodeDirective(const NodeDirective *node, SectionTable *sections) {
    if (strcmp(node->name, ".global") == 0) return; // early return as this has already been handled
    if (strcmp(node->name, ".extern") == 0) return;

    if (strcmp(node->name, ".section") == 0) {
        Section *sect = get_section(sections, node->args.tokens[0].value);

        u64 tmp1 = (u64)sect;
        u64 tmp2 = (u64)&sections->sections[0];
            u64 sect_idx = (tmp1 - tmp2) / sizeof (Section);

        sections->current = sect_idx;
        return;
    }

    u64 tok_idx = 0;
    Token *tok = &node->args.tokens[tok_idx];

    if (strcmp(node->name, ".db") == 0) {
        emit_define(node, &sections->sections[sections->current], 1);
        return;
    }

    if (strcmp(node->name, ".dw") == 0) {
        emit_define(node, &sections->sections[sections->current], 2);
        return;
    }

    if (strcmp(node->name, ".dd") == 0) {
        emit_define(node, &sections->sections[sections->current], 4);
        return;
    }

    if (strcmp(node->name, ".dq") == 0) {
        emit_define(node, &sections->sections[sections->current], 8);
        return;
    }

    if (strcmp(node->name, ".ascii") == 0) {
        while (tok_idx < node->args.count) {
            if (tok->type != TT_STRING) {
                error(tok->pos, "invalid argument for \"%s\" directive", node->name);
            }

            String *s = tok->value;

            push_bytes(&sections->sections[sections->current], (u8 *)s->str, s->size);

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

            push_bytes(&sections->sections[sections->current], (u8 *)s->str, s->size);
            push_bytes(&sections->sections[sections->current], (u8 *)&null_terminator, 1);

            tok = &node->args.tokens[++tok_idx];
        }

        return;
    }

    error(node->pos, "invalid directive");
}

void visit_NodeStatement(const NodeStatement *node, SectionTable *sections) {
    if (node->kind == ST_INSTRUCTION) {
        visit_NodeInstruction(&node->instruction, sections);
    } else if (node->kind == ST_DIRECTIVE) {
        visit_NodeDirective(&node->directive, sections);
    } else if (node->kind == ST_SYMBOL) {
        return;
    } else {
        error((Position){0}, "Excuse me what the actual fuck are you doing in my house?");
    }
}

void generate_code(NodeProgram *ast, SymbolTable *symbols, SectionTable *sections, RelocationTable *relocationTable) {
    if (!ast) return;

    symbol_pass(ast, symbols, sections, relocationTable);

    halt_on_error();

    for (u64 i = 0; i < ast->count; i++) {
        visit_NodeStatement(&ast->statements[i], sections);
    }
}
