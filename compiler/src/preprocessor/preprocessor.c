#include <stdlib.h>
#include <error.h>

#include "preprocessor.h"

Error preprocessor_init(Preprocessor *preprocessor, const Lexer *lexer) {
    if (!lexer || !preprocessor) return ERROR_NULL_POINTER;

    Error code = vector_init(&preprocessor->files, sizeof(Lexer));
    if (code != ERROR_OK) {
        return code;
    }

    code = vector_init(&preprocessor->expansions, sizeof(PPExpansionFrame));

    if (code != ERROR_OK) {
        vector_destroy(&preprocessor->files);
        return code;
    }

    code = vector_init(&preprocessor->macro_table, sizeof(Macro));

    if (code != ERROR_OK) {
        vector_destroy(&preprocessor->files);
        vector_destroy(&preprocessor->expansions);
        return code;
    }

    vector_push(&preprocessor->files, lexer);

    return ERROR_OK;
}

static Error pp_read_unexpanded(...);
static Error pp_peek_unexpanded(...);
static Error pp_process_directive(...);
static Error pp_try_expand_macro(...);
static Error pp_collect_arguments(...);
static Error pp_generate_replacement(...);
static Error pp_push_source(...);
static Error pp_is_active(...);

Error preprocessor_next(Preprocessor *preprocessor, PPToken *token, PreprocessorError *error) {
    if (!preprocessor || !token) {
        return ERROR_NULL_POINTER;
    }

    for (;;) {

    }

    return ERROR_INTERNAL;
}

void preprocessor_destroy(const Preprocessor *preprocessor) {

}
