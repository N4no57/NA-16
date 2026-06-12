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

bool collect_operands(Machine *machine, Instruction *inst, const u8 *inst_ops) { // current implementation 3 operand only
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
            u64 tmp;
            const bool success = fetch_byte(machine, &tmp);
            if (!success) return false;

            i8 value = (i8)tmp;
            inst->ops[0].displacement = (i16)value;
            inst->size++;
        } else {
            u8 bytes[2];
            u64 tmp;
            bool success = fetch_byte(machine, &tmp);
            if (!success) return false;

            bytes[0] = tmp & 0xFF;
            success = fetch_byte(machine, &tmp);
            if (!success) return false;

            bytes[1] = tmp & 0xFF;
            u16 u = (u16)bytes[0] | ((u16)bytes[1] << 8);
            inst->ops[0].displacement = (i16)u;
            inst->size += 2;
        }
        return true;
    }

    for (u64 i = 0; i < inst->op_count; i++) {
        switch (inst->ops[i].mode) {
            case OP_REG: // register
                inst->ops[i].reg = inst_ops[i];
                break;
            case OP_ABSOLUTE:
            case OP_IMM: // immediate
                u64 immediate;

                if (inst->ops[i].size == 1) {
                    const bool success = fetch_byte(machine, &immediate);
                    if (!success) return false;

                    inst->size++;
                } else {
                    const bool success = fetch_word(machine, &immediate);
                    if (!success) return false;

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
                    const bool success = fetch_byte(machine, &immediate);
                    if (!success) return false;

                    i8 tmp = (i8)immediate;
                    displacement = (i16)tmp;
                    inst->size++;
                } else {
                    const bool success = fetch_word(machine, &immediate);
                    if (!success) return false;

                    displacement = (i16)immediate;
                    inst->size += 2;
                }
                inst->ops[i].displacement = displacement;
                break;
            case OP_SIB:
                inst->ops[i].reg = inst_ops[i];
                bool success = fetch_byte(machine, &immediate);
                if (!success) return false;

                u8 SIB_block = immediate;
                inst->ops[i].scale = SIB_block >> 6 & 0x3;
                inst->ops[i].idx_reg = SIB_block >> 3 & 0x7;
                break;
            case OP_SIB_DISP:
                inst->ops[i].reg = inst_ops[i];
                success = fetch_byte(machine, &immediate);
                if (!success) return false;

                SIB_block = immediate;
                inst->ops[i].scale = SIB_block >> 6 & 0x3;
                inst->ops[i].idx_reg = SIB_block >> 3 & 0x7;
                if (inst->ops[i].size == 1) {
                    success = fetch_byte(machine, &immediate);
                    if (!success) return false;

                    const i8 tmp = (i8)immediate;
                    displacement = (i16)tmp;
                    inst->size++;
                } else {
                    success = fetch_word(machine, &immediate);
                    if (!success) return false;

                    displacement = (i16)immediate;
                    inst->size += 2;
                }
                inst->ops[i].displacement = displacement;
                break;
            default:
                printf("OGOHGOHGOH");
                return false;
        }
    }

    return true;
}

bool operand_read(Machine *machine, const Operand op, u64 *value) {
    u16 address;
    switch (op.mode) {
        case OP_REG:
            *value = read_reg(machine, op.reg);
            return true;

        case OP_IMM:
            *value = op.immediate;
            return true;

        case OP_REG_IND:
            if (op.size == 1) {
                const bool success = read_byte(machine, read_reg(machine, op.reg), value);
                if (!success) return false;
            } else {
                const bool success = read_word(machine, read_reg(machine, op.reg), value);
                if (!success) return false;
            }
            return true;

        case OP_ABSOLUTE:
            if (op.size == 1) {
                const bool success = read_byte(machine, op.immediate, value);
                if (!success) return false;
            } else {
                const bool success = read_word(machine, op.immediate,value);
                if (!success) return false;
            }
            return true;

        case OP_REG_IND_DISP:
            address = read_reg(machine, op.reg);
            address += op.displacement;
            if (op.size == 1) {
                const bool success = read_byte(machine, address, value);
                if (!success) return false;
            } else {
                const bool success = read_word(machine, address, value);
                if (!success) return false;
            }
            return true;

        case OP_SIB:
            address = read_reg(machine, op.reg);
            address += read_reg(machine, op.idx_reg) << op.scale;
            if (op.size == 1) {
                const bool success = read_byte(machine, address, value);
                if (!success) return false;
            } else {
                const bool success = read_word(machine, address, value);
                if (!success) return false;
            }
            return true;

        case OP_SIB_DISP:
            address = read_reg(machine, op.reg);
            address += read_reg(machine, op.idx_reg) << op.scale;
            address += op.displacement;
            if (op.size == 1) {
                const bool success = read_byte(machine, address, value);
                if (!success) return false;
            } else {
                const bool success = read_word(machine, address, value);
                if (!success) return false;
            }
            return true;
    }

    return false;
}

bool operand_write(Machine *machine, const Operand op, const u16 value) {
    u16 address;
    switch (op.mode) {
        case OP_REG:
            set_reg(machine, op.reg, value);
            return true;

        case OP_REG_IND:
            if (op.size == 1) {
                const bool success = write_byte(machine, read_reg(machine, op.reg), value);
                if (!success) return false;
            }
            else {
                const bool success = write_word(machine, read_reg(machine, op.reg), value);
                if (!success) return false;
            }
            return true;

        case OP_ABSOLUTE:
            if (op.size == 1) {
                const bool success = write_byte(machine, op.immediate, value);
                if (!success) return false;
            }
            else {
                const bool success = write_word(machine, op.immediate, value);
                if (!success) return false;
            }
            return true;

        case OP_REG_IND_DISP:
            address = read_reg(machine, op.reg);
            address += op.displacement;
            if (op.size == 1) {
                const bool success = write_byte(machine, address, value);
                if (!success) return false;
            }
            else {
                const bool success = write_word(machine, address, value);
                if (!success) return false;
            }
            return true;

        case OP_SIB:
            address = read_reg(machine, op.reg);
            address += read_reg(machine, op.idx_reg) << op.scale;
            if (op.size == 1) {
                const bool success = write_byte(machine, address, value);
                if (!success) return false;
            }
            else {
                const bool success = write_word(machine, address, value);
                if (!success) return false;
            }
            return true;

        case OP_SIB_DISP:
            address = read_reg(machine, op.reg);
            address += read_reg(machine, op.idx_reg) << op.scale;
            address += op.displacement;
            if (op.size == 1) {
                const bool success = write_byte(machine, address, value);
                if (!success) return false;
            }
            else {
                const bool success = write_word(machine, address, value);
                if (!success) return false;
            }
            return true;
    }

    return false;
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
    [INT] = {"INT", 1, int_handler, false},
    [IRET] = {"IRET", 0, iret_handler, true},

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

    u64 value;
    bool success = true;
    // OBTAIN PREFIXES
    while (1) {
        success = read_byte(machine, cpu->sys.PC, &value);
        if (!success) return false;

        u8 byte = value & 0xFF;
        if (!(byte & 0x80)) break;

        if ((byte & 0xF0) == 0x80) {
            success = fetch_word(machine, &value);
            if (!success) return false;

            const u16 MEX_prefix = value & 0xFFFF;
            inst->prefixes.MEX = (MEX_prefix & 0xFF) << 8 | MEX_prefix >> 8 & 0xFF;
            inst->size += 2;
            continue;
        }

        if ((byte & 0xF0) == 0x90) {
            success = fetch_byte(machine, &value);
            if (!success) return false;

            inst->prefixes.AEX = value;
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

    success = fetch_word(machine, &value);
    if (!success) return false;

    u16 instruction = value << 8 & 0xFF00;
    instruction |= value >> 8 & 0xFF;

    inst->opcode = instruction >> 9 & 0x7F;
    inst->size += 2;

    const InstructionDef *info = &instruction_table[inst->opcode];
    inst->op_count = info->operand_count;

    const u8 op[3] = {
        instruction >> 6 & 0x7,
        instruction >> 3 & 0x7,
        instruction & 0x7
    };

    success = collect_operands(machine, inst, op);
    if (!success) return false;

    return true;
}