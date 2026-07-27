#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "target/target.h"
#include "type.h"
#include "lexer/lexer.h"
#include "preprocessor/c_token.h"
#include "preprocessor/preprocessor.h"

int main(void) {
    SourceFile file = {0};

    source_file_load(&file, "test.c");

    Lexer lexer;
    Preprocessor preprocessor;

    lexer_init(&lexer, &file);
    preprocessor_init(&preprocessor, &lexer);

    PPToken pp_token;
    CToken c_token;
    LexerError error;
    while (preprocessor_next(&preprocessor, &pp_token, &error)) {
        if (!convert_ppt_to_ct(&pp_token, &c_token)) {
            break;
        }

        if (c_token.kind == C_TOKEN_EOF) {
            break;
        }
    }

    source_file_destroy(&file);

    return EXIT_SUCCESS;
}
