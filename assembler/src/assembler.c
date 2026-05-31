#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"
#include "codegen/codegen.h"
#include "instruction_selection/inst_selector.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic_analyser/analyser.h"
#include "lib/error.h"

void write_binary(const char *file, const bytes code) {
    FILE *f = fopen(file, "wb");

    fwrite(code.data, code.count, 1, f);

    fclose(f);
}

int assemble(const char *in, const char *out) {
    TokenList tokens;
    NodeProgram ast;
    char *program = read_assembly(in);

    tokenise(&tokens, (u8 *)in, (u8 *)program);

    parse(&ast, &tokens);

    analyse(&ast);

    lowerer(&ast);

    if (error_count > 0 ) {
        if (error_count == 1) printf("Found 1 error:\n");
        else printf("Found %d errors:\n", error_count);

        exit(EXIT_FAILURE);
    }

    bytes code;
    code.count = 0;
    code.size = 8;
    code.data = malloc(code.size);
    if (code.data == NULL) {
        free(program);
        return 1;
    }

    generate_code(&ast, &code);

    write_binary(out, code);

    free(code.data);
    free(program);

    return 0;
}