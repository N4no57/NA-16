#include "memory.h"
#include "../machine.h"
#include "../mmu/mmu.h"

bool read_byte(Machine *machine, const u16 address, u64 *value) {
    CPU *cpu = &machine->cpu;
    if (cpu->sys.FR.V  == 1) {
        u32 true_address = 0;
        bool success = translate(machine, address, &true_address);
        if (!success) return false;

        *value = machine->ram.memory[true_address];
        return true;
    }

    if (address >= machine->ram.memory_size) {
        // interrupt(cpu, GP);
        return false;
    }

    *value = machine->ram.memory[address];
    return true;
}

bool read_word(const Machine *machine, const u64 address, u64 *value) {
    const CPU *cpu = &machine->cpu;
    if (cpu->sys.FR.V  == 1) {
        u32 true_address;
        bool success = translate(machine, address, &true_address);
        if (!success) return false;
        if (true_address >= machine->ram.memory_size) {
            // interrupt(cpu, GP);
            return false;
        }

        *value = machine->ram.memory[true_address];

        success = translate(machine, address + 1, &true_address);
        if (!success) return false;
        if (true_address >= machine->ram.memory_size) {
            // interrupt(cpu, GP);
            return false;
        }

        *value |= machine->ram.memory[true_address];
        return true;
    }

    if (address+1 >= machine->ram.memory_size) {
        // interrupt(cpu, GP);
        return false;
    }

    *value = machine->ram.memory[address] | machine->ram.memory[address + 1] << 8;
    return true;
}

bool write_byte(Machine *machine, const u16 address, const u8 value) {
    const CPU *cpu = &machine->cpu;
    if (cpu->sys.FR.V  == 1) {
        u32 true_address = 0;
        const bool success = translate(machine, address, &true_address);
        if (!success) return false;

        if (true_address >= machine->ram.memory_size) {
            // interrupt(cpu, GP);
            return false;
        }

        machine->ram.memory[true_address] = value;
    } else {
        if (address >= machine->ram.memory_size) {
            // interrupt(cpu, GP);
            return false;
        }

        machine->ram.memory[address] = value;
    }

    return true;
}

bool write_word(const Machine *machine, const u16 address, const u16 value) {
    const CPU *cpu = &machine->cpu;
    if (cpu->sys.FR.V == 1) {
        u32 true_address = 0;
        bool success = translate(machine, address, &true_address);
        if (!success) return false;
        if (true_address >= machine->ram.memory_size) {
            // interrupt(cpu, GP);
            return false;
        }

        machine->ram.memory[true_address] = value & 0xFF;

        success = translate(machine, true_address + 1, &true_address);
        if (!success) return false;
        if (true_address >= machine->ram.memory_size) {
            // interrupt(cpu, GP);
            return false;
        }

        machine->ram.memory[true_address] = value >> 8 & 0xFF;
    } else {
        if (address+1 >= machine->ram.memory_size) {
            // interrupt(cpu, GP);
            return false;
        }

        machine->ram.memory[address] = value & 0xFF;
        machine->ram.memory[address + 1] = value >> 8 & 0xFF;
    }

    return true;
}

bool fetch_byte(Machine *machine, u64 *value) {
    if (machine->cpu.sys.FR.V == 1) {
        const bool success = is_executable(machine);
        if (!success) return false;
    }

    return read_byte(machine, machine->cpu.sys.PC++, value);
}

bool fetch_word(Machine *machine, u64 *value) {
    if (machine->cpu.sys.FR.V == 1) {
        bool success = is_executable(machine);
        if (!success) return false;
        machine->cpu.sys.PC++;
        success = is_executable(machine);
        if (!success) return false;
        machine->cpu.sys.PC++;
    } else {
        machine->cpu.sys.PC += 2;
    }

    return read_word(machine, machine->cpu.sys.PC-2, value);
}