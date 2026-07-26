#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "target/target.h"
#include "type.h"
#include "lexer/lexer.h"

int main(void) {
    SourceFile file = {0};

    source_file_load(&file, "test.c");

    Lexer lexer;

    lexer_init(&lexer, &file);

    PPToken tok;
    LexerError error;
    while (true) {
        lexer_next(&lexer, &tok, &error);

        if (tok.kind == PP_TOKEN_EOF) {
            break;
        }
    }

    source_file_destroy(&file);

    return EXIT_SUCCESS;
}
