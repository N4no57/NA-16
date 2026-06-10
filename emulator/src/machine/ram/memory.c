#include "memory.h"
#include "../machine.h"
#include "../cpu/cpu.h"

typedef enum {
    PT_PRESENT = 1 << 11,
    PT_WRITABLE = 1 << 10,
    PT_EXECUTABLE = 1 << 9,
    PT_USER_ACCESS = 1 << 8
} PageTableFlags;

typedef struct {
    u32 frame;
} PageTableEntry;

u32 translate(Machine *machine, const u32 vaddr) {
    CPU *cpu = &machine->cpu;
    PageTableEntry entry;
    if (cpu->sys.FR.V  == 1) {
        const u32 *tmp = (u32 *)&machine->ram.memory[machine->mmu.user_page_table + (vaddr >> 12) * sizeof(u32)];
        entry.frame = *tmp;

        if ((entry.frame & PT_PRESENT) != PT_PRESENT) {
            // interrupt(cpu, PF);
            return 0;
        }

        return entry.frame << 12 | vaddr & 0xFFF;
    }

    const u32 *tmp = (u32 *)&machine->ram.memory[machine->mmu.kernel_page_table + (vaddr >> 12) * sizeof(u32)];
    entry.frame = *tmp;

    if ((entry.frame & PT_PRESENT) != PT_PRESENT) {
        // interrupt(cpu, PF);
        return 0;
    }

    return entry.frame & 0xFFFFF000 | vaddr & 0xFFF;
}

u8 read_byte(Machine *machine, const u16 address) {
    CPU *cpu = &machine->cpu;
    if (cpu->sys.FR.V  == 1) {
        const u64 true_address = translate(machine, address);

        return machine->ram.memory[true_address];
    }

    if (address >= machine->ram.memory_size) {
        // interrupt(cpu, GP);
        return 0;
    }

    return machine->ram.memory[address];
}

u16 read_word(Machine *machine, const u16 address) {
    CPU *cpu = &machine->cpu;
    if (cpu->sys.FR.V  == 1) {
        u64 true_address = translate(machine, address);
        if (true_address >= machine->ram.memory_size) {
            // interrupt(cpu, GP);
            return 0;
        }

        u64 ret_val = machine->ram.memory[true_address];

        true_address = translate(machine, address + 1);
        if (true_address >= machine->ram.memory_size) {
            // interrupt(cpu, GP);
            return 0;
        }

        ret_val |= machine->ram.memory[true_address];
        return ret_val;
    }

    if (address+1 >= machine->ram.memory_size) {
        // interrupt(cpu, GP);
        return 0;
    }

    return machine->ram.memory[address] | machine->ram.memory[address + 1] << 8;
}

void write_byte(Machine *machine, const u16 address, const u8 value) {
    const CPU *cpu = &machine->cpu;
    if (cpu->sys.FR.V  == 1) {
        const u64 true_address = translate(machine, address);

        if (true_address >= machine->ram.memory_size) {
            // interrupt(cpu, GP);
            return;
        }

        machine->ram.memory[true_address] = value;
    } else {
        if (address >= machine->ram.memory_size) {
            // interrupt(cpu, GP);
            return;
        }

        machine->ram.memory[address] = value;
    }
}

void write_word(Machine *machine, const u16 address, const u16 value) {
    const CPU *cpu = &machine->cpu;
    if (cpu->sys.FR.V == 1) {
        u64 true_address = translate(machine, address);
        if (true_address >= machine->ram.memory_size) {
            // interrupt(cpu, GP);
            return;
        }

        machine->ram.memory[true_address] = value & 0xFF;

        true_address = translate(machine, true_address + 1);
        if (true_address >= machine->ram.memory_size) {
            // interrupt(cpu, GP);
            return;
        }

        machine->ram.memory[true_address] = value >> 8 & 0xFF;
    } else {
        if (address+1 >= machine->ram.memory_size) {
            // interrupt(cpu, GP);
            return;
        }

        machine->ram.memory[address] = value & 0xFF;
        machine->ram.memory[address + 1] = value >> 8 & 0xFF;
    }
}

void is_executable(Machine *machine) {
    PageTableEntry entry;
    if (machine->cpu.sys.FR.U == 1) {
        const u32 *tmp = (u32 *)&machine->ram.memory[machine->mmu.user_page_table + (machine->cpu.sys.PC >> 12) * sizeof(u32)];
        entry.frame = *tmp;

        if ((entry.frame & PT_EXECUTABLE) != PT_EXECUTABLE) {
            // interrupt(cpu, PF);
        }
    } else {
        const u32 *tmp = (u32 *)&machine->ram.memory[machine->mmu.kernel_page_table + (machine->cpu.sys.PC >> 12) * sizeof(u32)];
        entry.frame = *tmp;

        if ((entry.frame & PT_EXECUTABLE) != PT_EXECUTABLE) {
            // interrupt(cpu, PF);
        }
    }
}

u8 fetch_byte(Machine *machine) {
    if (machine->cpu.sys.FR.V == 1) {
        is_executable(machine);
    }

    return read_byte(machine, machine->cpu.sys.PC++);
}

u16 fetch_word(Machine *machine) {
    if (machine->cpu.sys.FR.V == 1) {
        is_executable(machine);
        machine->cpu.sys.PC++;
        is_executable(machine);
        machine->cpu.sys.PC++;
    } else {
        machine->cpu.sys.PC += 2;
    }

    return read_word(machine, machine->cpu.sys.PC-2);
}