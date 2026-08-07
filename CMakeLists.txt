cmake_minimum_required(VERSION 4.2)
project(NA-16)

set(CMAKE_C_STANDARD 23)

include_directories(C:/libs/clib/include)
link_directories(C:/libs/clib/bin)

add_compile_options(-Wall -Wextra -Wpedantic -Wconversion)

add_subdirectory(compiler)
add_subdirectory(assembler)
add_subdirectory(linker)
add_subdirectory(emulator)
