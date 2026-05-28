cmake_minimum_required(VERSION 4.2)
project(Emulator C)

set(CMAKE_C_STANDARD 23)

add_executable(Emulator src/main.c
        src/cpu/cpu.c src/cpu/cpu.h
        src/cpu/memory.c src/cpu/memory.h
        src/cpu/instructions/inst.h src/cpu/instructions/inst.c
        src/cpu/instructions/handlers/add.c
        src/cpu/instructions/handlers/sub.c
        src/cpu/instructions/handlers/mov.c
        src/cpu/instructions/handlers/jmp.c
)