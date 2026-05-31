cmake_minimum_required(VERSION 4.2)
project(Emulator C)

set(CMAKE_C_STANDARD 23)

add_executable(Emulator src/main.c
        src/cpu/cpu.c src/cpu/cpu.h
        src/cpu/memory.c src/cpu/memory.h
        src/cpu/instructions/inst.h src/cpu/instructions/inst.c

        src/cpu/instructions/handlers/ALU_ops/add.c
        src/cpu/instructions/handlers/ALU_ops/sub.c
        src/cpu/instructions/handlers/ALU_ops/and.c
        src/cpu/instructions/handlers/ALU_ops/not.c
        src/cpu/instructions/handlers/ALU_ops/or.c
        src/cpu/instructions/handlers/ALU_ops/xor.c
        src/cpu/instructions/handlers/ALU_ops/cmp.c
        src/cpu/instructions/handlers/ALU_ops/test.c

        src/cpu/instructions/handlers/data_movement/mov.c
        src/cpu/instructions/handlers/data_movement/movsr.c
        src/cpu/instructions/handlers/data_movement/movrs.c
        src/cpu/instructions/handlers/data_movement/push.c
        src/cpu/instructions/handlers/data_movement/pop.c
        src/cpu/instructions/handlers/data_movement/lea.c
        src/cpu/instructions/handlers/data_movement/pushs.c
        src/cpu/instructions/handlers/data_movement/movs.c
        src/cpu/instructions/handlers/data_movement/pops.c

        src/cpu/instructions/handlers/control_flow/jmp.c
        src/cpu/instructions/handlers/control_flow/jz.c
        src/cpu/instructions/handlers/control_flow/jnz.c
        src/cpu/instructions/handlers/control_flow/jc.c
        src/cpu/instructions/handlers/control_flow/jnc.c
        src/cpu/instructions/handlers/control_flow/jo.c
        src/cpu/instructions/handlers/control_flow/jno.c
        src/cpu/instructions/handlers/control_flow/js.c
        src/cpu/instructions/handlers/control_flow/jns.c
        src/cpu/instructions/handlers/control_flow/ja.c
        src/cpu/instructions/handlers/control_flow/jbe.c
        src/cpu/instructions/handlers/control_flow/jg.c
        src/cpu/instructions/handlers/control_flow/jge.c
        src/cpu/instructions/handlers/control_flow/jl.c
        src/cpu/instructions/handlers/control_flow/jle.c
        src/cpu/instructions/handlers/control_flow/call.c
)