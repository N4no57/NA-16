#include "mmu.h"

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
