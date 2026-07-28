#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "target/target.h"
#include "type.h"
#include "ir/ir.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "parser/token_stream.h"
#include "preprocessor/c_token.h"
#include "preprocessor/preprocessor.h"
#include "semantic_analyser/analyser.h"

int main(void) {
    SourceFile file = {0};

    source_file_load(&file, "test.c");

    Lexer lexer;
    Preprocessor preprocessor;
    TokenStream stream;
    Parser parser;

    lexer_init(&lexer, &file);
    preprocessor_init(&preprocessor, &lexer);
    token_stream_init(&stream, &preprocessor);
    parser_init(&parser, &stream);

    TranslationUnit unit;

    translation_unit_init(&unit);

    parser_parse_translation_unit(&parser, &unit);

    SemanticContext semantic_context = {
        .target = target_na16(),
        .current_function_return_type = nullptr
    };

    analyse_function_definition(&semantic_context ,&unit.items[0].data.function_definition);

    IRModule module = {
        .function_capacity = 8,
        .function_count = 0
    };

    module.functions = malloc(sizeof(IRFunction) * module.function_capacity);

    lower_ast(&unit, &module);

    translation_unit_destroy(&unit);
    source_file_destroy(&file);

    return EXIT_SUCCESS;
}
