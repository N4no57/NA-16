#include "machine.h"

#include "cpu/cpu.h"

void machine_step(Machine *machine) {
    cpu_step(machine);
}
