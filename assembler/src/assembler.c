#include <stdio.h>
#include <stdlib.h>

#include "assembler.h"
#include "codegen/codegen.h"
#include "instruction_selection/inst_selector.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic_analyser/analyser.h"
#include "lib/error.h"
#include "obj_file_writer/obj_writer.h"
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

int assemble(const char *in, char *out) {
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

    RelocationTable relocations;
    init_RelocationTable(&relocations);

    SymbolTable symbols;

    generate_code(&ast, &symbols, &sections, &relocations);

    ObjectFile obj = {0};
    obj.header.magic = -1;
    obj.header.version = 0;

    obj.header.section_table_size = sections.count;

    obj.header.symbol_table_size = symbols.count;

    obj.header.relocation_table_size = relocations.count;

    obj.section_table = sections.sections;
    obj.symbol_table = symbols.symbols;
    obj.relocation_table = relocations.relocations;

    write_obj(&obj, out);

    free_SectionTable(&sections);
    free(program);

    return 0;
}