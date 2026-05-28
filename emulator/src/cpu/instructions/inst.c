#include "inst.h"
#include "../memory.h"

bool has_MEX(const Instruction *inst) {
    return (inst->prefixes.MEX >> 8 & 0xF0) == 0x80 ? true : false;
}

bool has_AEX(const Instruction *inst) {
    return (inst->prefixes.AEX & 0xF0) == 0x90 ? true : false;
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

InstructionDef instruction_table[] = {
    [ADD] = {"ADD", 3, add_handler},
    [SUB] = {"SUB", 3, sub_handler},
    [MOV] = {"MOV", 2, mov_handler},
    [JMP] = {"JMP", 1, jmp_handler},
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