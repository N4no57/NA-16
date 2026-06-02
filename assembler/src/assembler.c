#include <stdio.h>
#include <stdlib.h>

#include "assembler.h"
#include "codegen/codegen.h"
#include "instruction_selection/inst_selector.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic_analyser/analyser.h"
#include "lib/error.h"
#include "preprocessor/preprocessor.h"

void write_binary(const char *file, SectionTable *sections) {
    FILE *f = fopen(file, "wb");

    for (u64 i =0; i < sections->count; i++) {
        Section *section = &sections->sections[i];
        if (section->count == 0) continue;

        fwrite(section->data, section->count, 1, f);
    }

    fclose(f);
}

int assemble(const char *in, const char *out) {
    TokenList tokens;
    NodeProgram ast;
    char *program = read_assembly(in);

    tokenise(&tokens, (u8 *)in, (u8 *)program);

    preprocessor(&tokens);

    halt_on_error();

    parse(&ast, &tokens);

    analyse(&ast);

    lowerer(&ast);

    halt_on_error();

    SectionTable sections;
    init_SectionTable(&sections);

    generate_code(&ast, &sections);

    write_binary(out, &sections);

    free_SectionTable(&sections);
    free(program);

    return 0;
}