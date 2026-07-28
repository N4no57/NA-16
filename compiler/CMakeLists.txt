CMAKE_MINIMUM_REQUIRED(VERSION 4.2)
project(Compiler C)

set(CMAKE_C_STANDARD 23)

add_compile_options(-Wall -Wextra -Wpedantic -Wconversion)

add_executable(Compiler src/main.c
        src/target/target.c src/target/target.h
        src/type.c src/type.h
        src/lexer/token.c src/lexer/token.h
        src/lexer/source.c src/lexer/source.h
        src/lexer/lexer.c src/lexer/lexer.h
        src/preprocessor/preprocessor.c src/preprocessor/preprocessor.h
        src/preprocessor/c_token.c src/preprocessor/c_token.h
        src/parser/token_stream.c src/parser/token_stream.h
        src/parser/ast.c src/parser/ast.h
        src/parser/parser.c src/parser/parser.h
        src/semantic_analyser/analyser.c
        src/semantic_analyser/analyser.h
        src/ir/ir.c
        src/ir/ir.h
)