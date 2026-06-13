#include "memory.h"
#include "../machine.h"
#include "../mmu/mmu.h"
#include "../PIC/pic.h"

bool resolve_address(Machine *machine, u64 virtual_address, u64 *physical_address) {
    CPU *cpu = &machine->cpu;
    if (cpu->sys.FR.V  == 1) {
        bool success = translate(machine, virtual_address, (u32 *)physical_address);
        if (!success) {
            raise_exception(machine, PF, virtual_address);
            return false;
        }

        if (*physical_address >= machine->ram.memory_size) {
            raise_exception(machine, GP, virtual_address);
            return false;
        }
    } else {
        if (virtual_address >= machine->ram.memory_size) {
            raise_exception(machine, GP, virtual_address);
            return false;
        }

        *physical_address = virtual_address;
    }

    return true;
}

bool read_byte(Machine *machine, const u16 address, u64 *value) {
    u64 physical_address;
    if (!resolve_address(machine, address, &physical_address)) return false;

    *value = machine->ram.memory[physical_address];
    return true;
}

bool read_word(Machine *machine, const u64 address, u64 *value) {
    u64 tmp;

    if (!read_byte(machine, address, value)) return false;
    if (!read_byte(machine, address + 1, &tmp)) return false;

    *value |= tmp << 8 & 0xFF00;

    return true;
}

bool write_byte(Machine *machine, const u16 address, const u8 value) {
    u64 physical_address;

    if (!resolve_address(machine, address, &physical_address)) return false;

    machine->ram.memory[address] = value;

    return true;
}

bool write_word(Machine *machine, const u16 address, const u16 value) {
    return write_byte(machine, address, value & 0xFF) &&
        write_byte(machine, address + 1, value >> 8 & 0xFF);
}

bool fetch_byte(Machine *machine, u64 *value) {
    if (machine->cpu.sys.FR.V == 1) {
        const bool success = is_executable(machine);
        if (!success) {
            raise_exception(machine, PF, machine->cpu.sys.PC);
            return false;
        }

    }

    return read_byte(machine, machine->cpu.sys.PC++, value);
}

bool fetch_word(Machine *machine, u64 *value) {
    if (machine->cpu.sys.FR.V == 1) {
        bool success = is_executable(machine);
        if (!success) {
            raise_exception(machine, PF, machine->cpu.sys.PC);
            return false;
        }

        machine->cpu.sys.PC++;
        success = is_executable(machine);
        if (!success) {
            raise_exception(machine, PF, machine->cpu.sys.PC);
            return false;
        }

        machine->cpu.sys.PC++;
    } else {
        machine->cpu.sys.PC += 2;
    }

    return read_word(machine, machine->cpu.sys.PC-2, value);
}