cmake_minimum_required(VERSION 4.2)
project(Assembler C)

set(CMAKE_C_STANDARD 23)

add_executable(Assembler src/main.c
        src/assembler.c src/assembler.h
        src/lexer/lexer.c src/lexer/lexer.h
        src/parser/parser.c src/parser/parser.h src/parser/ast.h
        src/semantic_analyser/analyser.c src/semantic_analyser/analyser.h
        src/instruction_selection/inst_selector.c src/instruction_selection/inst_selector.h
        src/codegen/codegen.c src/codegen/symbol_pass.c src/codegen/codegen.h
        src/lib/asmlib.c src/lib/isa.c src/lib/asmlib.h
        src/lib/error.c src/lib/error.h
        src/preprocessor/preprocessor.c src/preprocessor/preprocessor.h
        src/lexer/tokens.c
        src/lexer/tokens.h
        src/lib/sections.c
        src/lib/sections.h
        src/obj_file_writer/obj_writer.c
        src/obj_file_writer/obj_writer.h
)