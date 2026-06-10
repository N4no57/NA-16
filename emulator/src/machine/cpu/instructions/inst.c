#include "inst.h"

#include <stdio.h>
#include <string.h>

#include "../../ram/memory.h"

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

void collect_operands(Machine *machine, Instruction *inst, const u8 *inst_ops) { // current implementation 3 operand only
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
            i8 value = (i8)fetch_byte(machine);
            inst->ops[0].displacement = (i16)value;
            inst->size++;
        } else {
            u8 bytes[2];
            bytes[0] = fetch_byte(machine);
            bytes[1] = fetch_byte(machine);
            u16 u = (u16)bytes[0] | ((u16)bytes[1] << 8);
            inst->ops[0].displacement = (i16)u;
            inst->size += 2;
        }
        return;
    }

    for (u64 i = 0; i < inst->op_count; i++) {
        switch (inst->ops[i].mode) {
            case OP_REG: // register
                inst->ops[i].reg = inst_ops[i];
                break;
            case OP_ABSOLUTE:
            case OP_IMM: // immediate
                u16 immediate;

                if (inst->ops[i].size == 1) {
                    immediate = fetch_byte(machine);
                    inst->size++;
                } else {
                    immediate = fetch_word(machine);
                    inst->size += 2;
                }

                inst->ops[i].immediate = immediate;
                break;
            case OP_REG_IND:
                inst->ops[i].reg = inst_ops[i];
                break;
            case OP_REG_IND_DISP:
                inst->ops[i].reg = inst_ops[i];
                i16 displacement;
                if (inst->ops[i].size == 1) {
                    i8 tmp = (i8)fetch_byte(machine);
                    displacement = (i16)tmp;
                    inst->size++;
                } else {
                    displacement = (i16)fetch_word(machine);
                    inst->size += 2;
                }
                inst->ops[i].displacement = displacement;
                break;
            case OP_SIB:
                inst->ops[i].reg = inst_ops[i];
                u8 SIB_block = fetch_byte(machine);
                inst->ops[i].scale = SIB_block >> 6 & 0x3;
                inst->ops[i].idx_reg = SIB_block >> 3 & 0x7;
                break;
            case OP_SIB_DISP:
                inst->ops[i].reg = inst_ops[i];
                SIB_block = fetch_byte(machine);
                inst->ops[i].scale = SIB_block >> 6 & 0x3;
                inst->ops[i].idx_reg = SIB_block >> 3 & 0x7;
                if (inst->ops[i].size == 1) {
                    i8 tmp = (i8)fetch_byte(machine);
                    displacement = (i16)tmp;
                    inst->size++;
                } else {
                    displacement = (i16)fetch_word(machine);
                    inst->size += 2;
                }
                inst->ops[i].displacement = displacement;
                break;
            default:
                printf("OGOHGOHGOH");
                break;
        }
    }
}

u16 operand_read(Machine *machine, const Operand op) {
    u16 value;
    u16 address;
    switch (op.mode) {
        case OP_REG:
            return read_reg(machine, op.reg);

        case OP_IMM:
            return op.immediate;

        case OP_REG_IND:
            if (op.size == 1) value = read_byte(machine, read_reg(machine, op.reg));
            else value = read_word(machine, read_reg(machine, op.reg));
            return value;

        case OP_ABSOLUTE:
            if (op.size == 1) value = read_byte(machine, op.immediate);
            else value = read_word(machine, op.immediate);
            return value;

        case OP_REG_IND_DISP:
            address = read_reg(machine, op.reg);
            address += op.displacement;
            if (op.size == 1) value = read_byte(machine, address);
            else value = read_word(machine, address);
            return value;

        case OP_SIB:
            address = read_reg(machine, op.reg);
            address += read_reg(machine, op.idx_reg) << op.scale;
            if (op.size == 1) value = read_byte(machine, address);
            else value = read_word(machine, address);
            return value;

        case OP_SIB_DISP:
            address = read_reg(machine, op.reg);
            address += read_reg(machine, op.idx_reg) << op.scale;
            address += op.displacement;
            if (op.size == 1) value = read_byte(machine, address);
            else value = read_word(machine, address);
            return value;
    }

    return 0;
}

void operand_write(Machine *machine, const Operand op, const u16 value) {
    u16 address;
    switch (op.mode) {
        case OP_REG:
            set_reg(machine, op.reg, value);
            break;

        case OP_REG_IND:
            if (op.size == 1) write_byte(machine, read_reg(machine, op.reg), value);
            else write_word(machine, read_reg(machine, op.reg), value);
            break;

        case OP_ABSOLUTE:
            if (op.size == 1) write_byte(machine, op.immediate, value);
            else write_word(machine, op.immediate, value);
            break;

        case OP_REG_IND_DISP:
            address = read_reg(machine, op.reg);
            address += op.displacement;
            if (op.size == 1) write_byte(machine, address, value);
            else write_word(machine, address, value);
            break;

        case OP_SIB:
            address = read_reg(machine, op.reg);
            address += read_reg(machine, op.idx_reg) << op.scale;
            if (op.size == 1) write_byte(machine, address, value);
            else write_word(machine, address, value);
            break;

        case OP_SIB_DISP:
            address = read_reg(machine, op.reg);
            address += read_reg(machine, op.idx_reg) << op.scale;
            address += op.displacement;
            if (op.size == 1) write_byte(machine, address, value);
            else write_word(machine, address, value);
            break;
    }
}

void set_flags(Machine *machine, const u32 value, const u32 values[2], const u8 mask, const u8 size) {
    CPU *cpu = &machine->cpu;

    // mask = 0b0000, 1 - O, 2 - C, 3 - N, 4 - Z
    u32 width_mask = size == 4 ? 0xFFFFFFFF : size == 2 ? 0xFFFF : 0xFF;

    u32 sign_bit = 1u << (size * 8 - 1);

    if (mask & 0b1) cpu->sys.FR.Z = (value & width_mask) == 0;

    if (mask & 0b10) cpu->sys.FR.N = (value & sign_bit) != 0;

    if (mask & 0b100) cpu->sys.FR.C = value > width_mask;

    if (mask & 0b1000) {
        u32 a = values[0];
        u32 b = values[1];
        u32 r = value;

        cpu->sys.FR.O = (~(a ^ b) & (a ^ r) & sign_bit) != 0;
    }
}

InstructionDef instruction_table[] = {
    // op table 1
    // class 0: ALU ops
    [ADD] = {"ADD", 3, add_handler, false},
    [SUB] = {"SUB", 3, sub_handler, false},
    [AND] = {"AND", 3, and_handler, false},
    [OR] = {"OR", 3, or_handler, false},
    [XOR] = {"XOR", 3, xor_handler, false},
    [NOT] = {"NOT", 2, not_handler, false},
    [CMP] = {"CMP", 2, cmp_handler, false},
    [TEST] = {"TEST", 2, test_handler, false},

    // class 1: data movement
    [MOV] = {"MOV", 2, mov_handler, false},
    [MOVSR] = {"MOVSR", 2, movsr_handler, false},
    [MOVRS] = {"MOVRS", 2, movrs_handler, false},
    [PUSH] = {"PUSH", 1, push_handler, false},
    [POP] = {"POP", 1, pop_handler, false},
    [LEA] = {"LEA", 2, lea_handler, false},
    [MOVS] = {"MOVS", 2, movs_handler, false},
    [PUSHS] = {"PUSHS", 1, pushs_handler, false},
    [POPS] = {"POPS", 1, pops_handler, false},

    // class 2: control flow
    [JMP] = {"JMP", 1, jmp_handler, false},
    [JZ] = {"JZ", 1, jz_handler, false},
    [JNZ] = {"JNZ", 1, jnz_handler, false},
    [JC] = {"JC", 1, jc_handler, false},
    [JNC] = {"JNC", 1, jnc_handler, false},
    [JO] = {"JO", 1, jo_handler, false},
    [JNO] = {"JNO", 1, jno_handler, false},
    [JS] = {"JS", 1, js_handler, false},
    [JNS] = {"JNS", 1, jns_handler, false},
    [JA] = {"JA", 1, ja_handler, false},
    [JBE] = {"JBE", 1, jbe_handler, false},
    [JG] = {"JG", 1, jg_handler, false},
    [JGE] = {"JGE", 1, jge_handler, false},
    [JL] = {"JL", 1, jl_handler, false},
    [JLE] = {"JLE", 1, jle_handler, false},
    [RET] = {"RET", 0, ret_handler, false},

    // class 3: system instructions
    [NOP] = {"NOP", 1, nullptr, false},
    [HLT] = {"HLT", 1, nullptr, false},

    // op table 2
    // class 0: ALU ops

    // class 1: data movement
    [CALL] = {"CALL", 1, call_handler, false},

    // class 2: control flow

    // class 3: system instructions
};

InstructionDef *fetch_InstDef(const Ops idx, bool has_escape_byte) {
    if (has_escape_byte) return &instruction_table[idx+0x100];
    return &instruction_table[idx];
}

bool decode(Machine *machine, Instruction *inst) {
    CPU *cpu = &machine->cpu;
    memset(inst, 0, sizeof(Instruction));

    // OBTAIN PREFIXES
    while (1) {
        const u8 byte = read_byte(machine, cpu->sys.PC);
        if (!(byte & 0x80)) break;

        if ((byte & 0xF0) == 0x80) {
            const u16 MEX_prefix = fetch_word(machine);
            inst->prefixes.MEX = (MEX_prefix & 0xFF) << 8 | MEX_prefix >> 8 & 0xFF;
            inst->size += 2;
            continue;
        }
        if ((byte & 0xF0) == 0x90) {
            inst->prefixes.AEX = fetch_byte(machine);
            inst->size++;
            continue;
        }

        if ((byte & 0xF0) == 0xF0) {
            inst->prefixes.has_escape_byte = true;
            cpu->sys.PC++;
            inst->size++;
            continue;
        }
    }

    u16 instruction = fetch_byte(machine) << 8;
    instruction |= fetch_byte(machine);

    inst->opcode = instruction >> 9 & 0x7F;
    inst->size += 2;

    const InstructionDef *info = &instruction_table[inst->opcode];
    inst->op_count = info->operand_count;

    const u8 op[3] = {
        instruction >> 6 & 0x7,
        instruction >> 3 & 0x7,
        instruction & 0x7
    };

    collect_operands(machine, inst, op);

    return true;
}