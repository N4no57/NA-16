CMAKE_MINIMUM_REQUIRED(VERSION 4.2)
project(Compiler C)

set(CMAKE_C_STANDARD 23)

add_compile_options(-Wall -Wextra -Wpedantic -Wconversion)

add_executable(Compiler src/main.c
        src/target/target.c
        src/target/target.h
        src/type.c
        src/type.h
        src/lexer/token.c
        src/lexer/token.h
        src/lexer/source.c
        src/lexer/source.h
        src/lexer/lexer.c
        src/lexer/lexer.h
)