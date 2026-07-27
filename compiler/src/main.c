#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "target/target.h"
#include "type.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "parser/token_stream.h"
#include "preprocessor/c_token.h"
#include "preprocessor/preprocessor.h"

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

    translation_unit_destroy(&unit);
    source_file_destroy(&file);

    return EXIT_SUCCESS;
}
