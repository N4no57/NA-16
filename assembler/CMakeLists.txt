cmake_minimum_required(VERSION 4.2)
project(Assembler C)

set(CMAKE_C_STANDARD 23)

add_executable(Assembler src/main.c
        src/lexer/lexer.c src/lexer/lexer.h
        src/lib/asmlib.c src/lib/asmlib.h
        src/parser/parser.c src/parser/parser.h
        src/parser/ast.h
        "src/semantic analyser/analyser.c" "src/semantic analyser/analyser.h"
        src/codegen/codegen.c src/codegen/codegen.h
        src/lib/error.c src/lib/error.h
        src/assembler.c src/assembler.h
        src/codegen/symbol_pass.c
        src/lib/isa.c
)