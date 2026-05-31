#include "inst.h"
#include "../memory.h"

bool has_MEX(const Instruction *inst) {
    return (inst->prefixes.MEX >> 8 & 0xF0) == 0x80 ? true : false;
}

bool has_AEX(const Instruction *inst) {
    return (inst->prefixes.AEX & 0xF0) == 0x90 ? true : false;
}

bool is_cond_jump(const Instruction *inst) {
    if (inst->opcode >= JZ && inst->opcode <= JLE) return true;

    return false;
}

void collect_operands(CPU *cpu, Instruction *inst, const u8 *inst_ops) { // current implementation 3 operand only
    if (has_MEX(inst)) {
        for (u8 i = 0; i < inst->op_count; i++) {
            inst->ops[i].mode = inst->prefixes.MEX >> (8 - 4 * i) & 0xF;
        }
    } else {
        for (u8 i = 0; i < inst->op_count; i++) {
            inst->ops[i].mode = 0;
        }
    }

    if (has_AEX(inst)) {
        for (u8 i = 0; i < inst->op_count; i++) {
            inst->ops[i].size = (inst->prefixes.AEX & 0xF) == 1 ? 2 : 1;
        }
    } else {
        for (u8 i = 0; i < inst->op_count; i++) {
            inst->ops[i].size = 1;
        }
    }

    if (is_cond_jump(inst)) {
        if (inst->ops[0].size == 1) {
            i8 value = (i8)fetch_byte(cpu);
            inst->ops[0].displacement = (i16)value;
        } else {
            u8 bytes[2];
            bytes[0] = fetch_byte(cpu);
            bytes[1] = fetch_byte(cpu);
            u16 u = (u16)bytes[0] | ((u16)bytes[1] << 8);
            inst->ops[0].displacement = (i16)u;
        }
        return;
    }

    for (u64 i = 0; i < inst->op_count; i++) {
        switch (inst->ops[i].mode) {
            case OP_REG: // register
                inst->ops[i].reg = inst_ops[i];
                break;
            case OP_IMM: // immediate
                u16 immediate;
                if (inst->ops[i].size == 1) immediate = fetch_byte(cpu);
                else immediate = fetch_word(cpu);
                inst->ops[i].immediate = immediate;
                break;
            case OP_REG_IND:
                inst->ops[i].reg = inst_ops[i];
                break;
            default:
                break;
        }
    }
}

u16 operand_read(const CPU *cpu, const Operand op) {
    switch (op.mode) {
        case OP_REG:
            return read_reg(cpu, op.reg);

        case OP_IMM:
            return op.immediate;

        case OP_REG_IND:
            u16 value;
            if (op.size == 1) value = read_byte(cpu, read_reg(cpu, op.reg));
            else value = read_word(cpu, read_reg(cpu, op.reg));
            return value;
    }

    return 0;
}

void operand_write(CPU *cpu, const Operand op, const u16 value) {
    if (op.mode == OP_REG) {
        set_reg(cpu, op.reg, value);
        return;
    }

    if (op.mode == OP_REG_IND) {
        if (op.size == 1) write_byte(cpu, read_reg(cpu, op.reg), value);
        else write_word(cpu, read_reg(cpu, op.reg), value);
        return;
    }
}

void set_flags(CPU *cpu, const u32 value, const u32 values[2], const u8 mask, u8 size) {
    // mask = 0b0000, 1 - O, 2 - C, 3 - N, 4 - Z
    u32 width_mask = size == 4 ? 0xFFFFFFFF : size == 2 ? 0xFFFF : 0xFF;

    u32 sign_bit = 1u << (size * 8 - 1);

    if (mask & 0b1) cpu->FR.Z = (value & width_mask) == 0;

    if (mask & 0b10) cpu->FR.N = (value & sign_bit) != 0;

    if (mask & 0b100) cpu->FR.C = value > width_mask;

    if (mask & 0b1000) {
        u32 a = values[0];
        u32 b = values[1];
        u32 r = value;

        cpu->FR.O = (~(a ^ b) & (a ^ r) & sign_bit) != 0;
    }
}

InstructionDef instruction_table[] = {
    // class 0: ALU ops
    [ADD] = {"ADD", 3, add_handler},
    [SUB] = {"SUB", 3, sub_handler},
    [AND] = {"AND", 3, and_handler},
    [OR] = {"OR", 3, or_handler},
    [XOR] = {"XOR", 3, xor_handler},
    [NOT] = {"NOT", 2, not_handler},

    // class 1: data movement
    [MOV] = {"MOV", 2, mov_handler},
    [MOVSR] = {"MOVSR", 2, movsr_handler},
    [MOVRS] = {"MOVRS", 2, movrs_handler},
    [PUSH] = {"PUSH", 1, push_handler},
    [POP] = {"POP", 1, pop_handler},
    [LEA] = {"LEA", 2, lea_handler},
    [MOVS] = {"MOVS", 2, movs_handler},
    [PUSHS] = {"PUSHS", 1, pushs_handler},
    [POPS] = {"POPS", 1, pops_handler},

    // class 2: control flow
    [JMP] = {"JMP", 1, jmp_handler},
    [JZ] = {"JZ", 1, jz_handler},
    [JNZ] = {"JNZ", 1, jnz_handler},
    [JC] = {"JC", 1, jc_handler},
    [JNC] = {"JNC", 1, jnc_handler},
    [JO] = {"JO", 1, jo_handler},
    [JNO] = {"JNO", 1, jno_handler},
    [JS] = {"JS", 1, js_handler},
    [JNS] = {"JNS", 1, jns_handler},
    [JA] = {"JA", 1, ja_handler},
    [JBE] = {"JBE", 1, jbe_handler},
    [JG] = {"JG", 1, jg_handler},
    [JGE] = {"JGE", 1, jge_handler},
    [JL] = {"JL", 1, jl_handler},
    [JLE] = {"JLE", 1, jle_handler},

    // class 3: system instructions
    [NOP] = {"NOP", 1, nullptr},
    [HLT] = {"HLT", 1, nullptr},
};

InstructionDef *fetch_InstDef(const Ops idx) {
    return &instruction_table[idx];
}

Instruction decode(CPU *cpu) {
    Instruction ret = {{0}, 255, {0}, 0};

    // OBTAIN PREFIXES
    while (1) {
        const u8 byte = read_byte(cpu, cpu->PC);
        if (!(byte & 0x80)) break;

        if ((byte & 0xF0) == 0x80) {
            const u16 MEX_prefix = fetch_word(cpu);
            ret.prefixes.MEX = (MEX_prefix & 0xFF) << 8 | MEX_prefix >> 8 & 0xFF;
            continue;
        }
        if ((byte & 0xF0) == 0x90) {
            ret.prefixes.AEX = fetch_byte(cpu);
            continue;
        }

        if ((byte & 0xF0) == 0xF0) {
            ret.prefixes.has_escape_byte = true;
            cpu->PC++;
            continue;
        }
    }

    u16 instruction = fetch_byte(cpu) << 8;
    instruction |= fetch_byte(cpu);

    ret.opcode = instruction >> 9 & 0x7F;

    const InstructionDef *inst = &instruction_table[ret.opcode];
    ret.op_count = inst->operand_count;

    const u8 op[3] = {
        instruction >> 6 & 0x7,
        instruction >> 3 & 0x7,
        instruction & 0x7
    };

    collect_operands(cpu, &ret, op);

    return ret;
}