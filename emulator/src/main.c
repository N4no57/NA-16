#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "machine/machine.h"
#include "machine/cpu/cpu.h"

void run_emulator(Machine *machine) {
    while (machine->powered_on) {
        execute_inst(machine);
    }
}

int main() {
    Machine machine = {0};
    machine.ram.memory_size = 0x10000;
    machine.ram.memory = malloc(machine.ram.memory_size);
    machine.powered_on = 1;

    cpu_init(&machine);
    machine.ram.memory[0xFFFE] = 0x00;
    machine.ram.memory[0xFFFF] = 0x00;
    cpu_reset(&machine);

    FILE *f = fopen("test.bin", "rb");
    fseek(f, 0, SEEK_END);
    const u64 size = ftell(f);
    fseek(f, 0, SEEK_SET);
    u8 *program = malloc(size);
    memset(program, 0, size);
    fread(program, 1, size, f);
    fclose(f);

    memcpy(&machine.ram.memory[0x0000], program, size);

    free(program);

    run_emulator(&machine);

    free(machine.ram.memory);
}