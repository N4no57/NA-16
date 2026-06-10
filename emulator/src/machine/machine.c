#include "machine.h"

#include "cpu/cpu.h"

void machine_step(Machine *machine) {
    execute_inst(machine);
}
