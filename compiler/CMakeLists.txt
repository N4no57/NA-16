CMAKE_MINIMUM_REQUIRED(VERSION 4.2)
project(Compiler C)

set(CMAKE_C_STANDARD 23)

add_compile_options(-Wall -Wextra -Wpedantic -Wconversion)

add_executable(Compiler src/main.c
        src/target.c
        src/target.h
)