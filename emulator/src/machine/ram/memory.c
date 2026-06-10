#include "memory.h"
#include "../machine.h"
#include "../mmu/mmu.h"

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