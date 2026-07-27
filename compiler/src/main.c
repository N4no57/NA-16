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
    ParserError parser_error;

    parser_parse_translation_unit(&parser, &unit, &parser_error);

    source_file_destroy(&file);

    return EXIT_SUCCESS;
}
