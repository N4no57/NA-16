#include "cpu/memory.h"

typedef enum {
    PT_PRESENT = 1 << 11,
    PT_WRITABLE = 1 << 10,
    PT_EXECUTABLE = 1 << 9,
    PT_USER_ACCESS = 1 << 8
} PageTableFlags;

typedef struct {
    u32 frame;
} PageTableEntry;

u32 translate(CPU *cpu, u32 vaddr) {
    PageTableEntry entry;
    if (cpu->FR.U == 1) {
        u32 *tmp = (u32 *)&cpu->memory[cpu->CR1 + (vaddr >> 12) * sizeof(u32)];
        entry.frame = *tmp;

        if ((entry.frame & PT_PRESENT) != PT_PRESENT) {
            interrupt(cpu, PF);
            return 0;
        }

        return entry.frame << 12 | vaddr & 0xFFF;
    }

    u32 *tmp = (u32 *)&cpu->memory[cpu->CR0 + (vaddr >> 12) * sizeof(u32)];
    entry.frame = *tmp;

    if ((entry.frame & PT_PRESENT) != PT_PRESENT) {
        interrupt(cpu, PF);
        return 0;
    }

    return entry.frame & 0xFFFFF000 | vaddr & 0xFFF;
}

u8 read_byte(CPU *cpu, const u16 address) {
    if (cpu->FR.V == 1) {
        u64 old_interrupt_count = interrupt_count;
        u64 true_address = translate(cpu, address);

        if (old_interrupt_count != interrupt_count) return 0;

        if (true_address >= cpu->memory_size) {
            interrupt(cpu, GP);
            return 0;
        }

        return cpu->memory[true_address];
    }

    if (address >= cpu->memory_size) {
        interrupt(cpu, GP);
        return 0;
    }

    return cpu->memory[address];
}

u16 read_word(CPU *cpu, const u16 address) {
    if (cpu->FR.V == 1) {
        u64 old_interrupt_count = interrupt_count;

        u64 true_address = translate(cpu, address);
        if (old_interrupt_count != interrupt_count) return 0;
        if (true_address >= cpu->memory_size) {
            interrupt(cpu, GP);
            return 0;
        }

        u64 ret_val = cpu->memory[true_address];

        true_address = translate(cpu, address + 1);
        if (old_interrupt_count != interrupt_count) return 0;
        if (true_address >= cpu->memory_size) {
            interrupt(cpu, GP);
            return 0;
        }

        ret_val |= cpu->memory[true_address];
        return ret_val;
    }

    if (address+1 >= cpu->memory_size) {
        interrupt(cpu, GP);
        return 0;
    }

    return cpu->memory[address] | cpu->memory[address + 1] << 8;
}

void write_byte(CPU *cpu, const u16 address, const u8 value) {
    if (cpu->FR.V == 1) {
        u64 old_interrupt_count = interrupt_count;

        u64 true_address = translate(cpu, address);
        if (old_interrupt_count != interrupt_count) return;

        if (true_address >= cpu->memory_size) {
            interrupt(cpu, GP);
            return;
        }

        cpu->memory[true_address] = value;
    } else {
        if (address >= cpu->memory_size) {
            interrupt(cpu, GP);
            return;
        }

        cpu->memory[address] = value;
    }
}

void write_word(CPU *cpu, const u16 address, const u16 value) {
    if (cpu->FR.V == 1) {
        u64 old_interrupt_count = interrupt_count;

        u64 true_address = translate(cpu, address);
        if (old_interrupt_count != interrupt_count) return;
        if (true_address >= cpu->memory_size) {
            interrupt(cpu, GP);
            return;
        }

        cpu->memory[true_address] = value & 0xFF;

        true_address = translate(cpu, true_address + 1);
        if (old_interrupt_count != interrupt_count) return;
        if (true_address >= cpu->memory_size) {
            interrupt(cpu, GP);
            return;
        }

        cpu->memory[true_address] = value >> 8 & 0xFF;
    } else {
        if (address+1 >= cpu->memory_size) {
            interrupt(cpu, GP);
            return;
        }

        cpu->memory[address] = value & 0xFF;
        cpu->memory[address + 1] = value >> 8 & 0xFF;
    }
}

void is_executable(CPU *cpu) {
    PageTableEntry entry;
    if (cpu->FR.U == 1) {
        u32 *tmp = (u32 *)&cpu->memory[cpu->CR1 + (cpu->PC >> 12) * sizeof(u32)];
        entry.frame = *tmp;

        if ((entry.frame & PT_EXECUTABLE) != PT_EXECUTABLE) {
            interrupt(cpu, PF);
        }
    } else {
        u32 *tmp = (u32 *)&cpu->memory[cpu->CR0 + (cpu->PC >> 12) * sizeof(u32)];
        entry.frame = *tmp;

        if ((entry.frame & PT_EXECUTABLE) != PT_EXECUTABLE) {
            interrupt(cpu, PF);
        }
    }
}

u8 fetch_byte(CPU *cpu) {
    u64 old_interrupt_count = interrupt_count;
    if (cpu->FR.V == 1) {
        is_executable(cpu);
        if (old_interrupt_count != interrupt_count) return 0;
    }

    return read_byte(cpu, cpu->PC++);
}

u16 fetch_word(CPU *cpu) {
    if (cpu->FR.V == 1) {
        u64 old_interrupt_count = interrupt_count;

        is_executable(cpu);
        if (old_interrupt_count != interrupt_count) return 0;

        cpu->PC++;
        is_executable(cpu);
        if (old_interrupt_count != interrupt_count) return 0;

        cpu->PC++;
    } else {
        cpu->PC += 2;
    }

    return read_word(cpu, cpu->PC-2);
}