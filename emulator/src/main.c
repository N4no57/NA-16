#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu/cpu.h"

int main() {
    CPU cpu;
    cpu_init(&cpu);
    cpu.memory[0xFFFE] = 0x00;
    cpu.memory[0xFFFF] = 0xF0;
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
    cpu.PC = 0x0000;

    free(program);

    while (1) {
        execute(&cpu);
    }
}