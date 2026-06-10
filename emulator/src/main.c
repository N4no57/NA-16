#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu/cpu.h"

void run_emulator(CPU *cpu) {
    while (1) {
        execute_inst(cpu);
    }
}

int main() {
    CPU cpu;
    cpu.memory_size = 0x10000;
    cpu.memory = malloc(cpu.memory_size);
    cpu_init(&cpu);
    cpu.memory[0xFFFE] = 0x00;
    cpu.memory[0xFFFF] = 0x00;
    cpu_reset(&cpu);

    FILE *f = fopen("test.bin", "rb");
    fseek(f, 0, SEEK_END);
    u64 size = ftell(f);
    fseek(f, 0, SEEK_SET);
    u8 *program = malloc(size);
    memset(program, 0, size);
    fread(program, 1, size, f);
    fclose(f);

    memcpy(&cpu.memory[0x0000], program, size);

    free(program);

    run_emulator(&cpu);

    free(cpu.memory);
}