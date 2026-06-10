cmake_minimum_required(VERSION 4.2)
project(Emulator C)

set(CMAKE_C_STANDARD 23)

add_executable(Emulator src/main.c
        src/machine/cpu/cpu.c src/machine/cpu/cpu.h
        src/machine/ram/memory.c src/machine/ram/memory.h
        src/machine/cpu/instructions/inst.h src/machine/cpu/instructions/inst.c

        src/machine/cpu/instructions/handlers/ALU_ops/add.c
        src/machine/cpu/instructions/handlers/ALU_ops/sub.c
        src/machine/cpu/instructions/handlers/ALU_ops/and.c
        src/machine/cpu/instructions/handlers/ALU_ops/not.c
        src/machine/cpu/instructions/handlers/ALU_ops/or.c
        src/machine/cpu/instructions/handlers/ALU_ops/xor.c
        src/machine/cpu/instructions/handlers/ALU_ops/cmp.c
        src/machine/cpu/instructions/handlers/ALU_ops/test.c

        src/machine/cpu/instructions/handlers/data_movement/mov.c
        src/machine/cpu/instructions/handlers/data_movement/movsr.c
        src/machine/cpu/instructions/handlers/data_movement/movrs.c
        src/machine/cpu/instructions/handlers/data_movement/push.c
        src/machine/cpu/instructions/handlers/data_movement/pop.c
        src/machine/cpu/instructions/handlers/data_movement/lea.c
        src/machine/cpu/instructions/handlers/data_movement/pushs.c
        src/machine/cpu/instructions/handlers/data_movement/movs.c
        src/machine/cpu/instructions/handlers/data_movement/pops.c

        src/machine/cpu/instructions/handlers/control_flow/jmp.c
        src/machine/cpu/instructions/handlers/control_flow/jz.c
        src/machine/cpu/instructions/handlers/control_flow/jnz.c
        src/machine/cpu/instructions/handlers/control_flow/jc.c
        src/machine/cpu/instructions/handlers/control_flow/jnc.c
        src/machine/cpu/instructions/handlers/control_flow/jo.c
        src/machine/cpu/instructions/handlers/control_flow/jno.c
        src/machine/cpu/instructions/handlers/control_flow/js.c
        src/machine/cpu/instructions/handlers/control_flow/jns.c
        src/machine/cpu/instructions/handlers/control_flow/ja.c
        src/machine/cpu/instructions/handlers/control_flow/jbe.c
        src/machine/cpu/instructions/handlers/control_flow/jg.c
        src/machine/cpu/instructions/handlers/control_flow/jge.c
        src/machine/cpu/instructions/handlers/control_flow/jl.c
        src/machine/cpu/instructions/handlers/control_flow/jle.c
        src/machine/cpu/instructions/handlers/control_flow/call.c
        src/machine/cpu/instructions/handlers/control_flow/ret.c
        src/machine/machine.h
        src/machine/mmu/mmu.c
        src/machine/mmu/mmu.h
)