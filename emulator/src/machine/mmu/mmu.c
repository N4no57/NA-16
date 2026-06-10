#include "mmu.h"

bool translate(const Machine *machine, const u32 vaddr, u32 *address) {
    const CPU *cpu = &machine->cpu;
    PageTableEntry entry;
    if (cpu->sys.FR.V  == 1) {
        const u32 *tmp = (u32 *)&machine->ram.memory[machine->mmu.user_page_table + (vaddr >> 12) * sizeof(u32)];
        entry.frame = *tmp;

        if ((entry.frame & PT_PRESENT) != PT_PRESENT) {
            // interrupt(cpu, PF);
            return false;
        }

        *address = entry.frame & 0xFFFFF000 | vaddr & 0xFFF;
        return true;
    }

    const u32 *tmp = (u32 *)&machine->ram.memory[machine->mmu.kernel_page_table + (vaddr >> 12) * sizeof(u32)];
    entry.frame = *tmp;

    if ((entry.frame & PT_PRESENT) != PT_PRESENT) {
        // interrupt(cpu, PF);
        return false;
    }

    *address = entry.frame & 0xFFFFF000 | vaddr & 0xFFF;
    return true;
}

bool is_executable(const Machine *machine) {
    PageTableEntry entry;
    if (machine->cpu.sys.FR.U == 1) {
        const u32 *tmp = (u32 *)&machine->ram.memory[machine->mmu.user_page_table + (machine->cpu.sys.PC >> 12) * sizeof(u32)];
        entry.frame = *tmp;

        if ((entry.frame & PT_EXECUTABLE) != PT_EXECUTABLE) {
            // interrupt(cpu, PF);
            return false;
        }
    } else {
        const u32 *tmp = (u32 *)&machine->ram.memory[machine->mmu.kernel_page_table + (machine->cpu.sys.PC >> 12) * sizeof(u32)];
        entry.frame = *tmp;

        if ((entry.frame & PT_EXECUTABLE) != PT_EXECUTABLE) {
            // interrupt(cpu, PF);
            return false;
        }
    }

    return true;
}
