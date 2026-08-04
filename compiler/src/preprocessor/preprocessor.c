#include <stdlib.h>
#include <error.h>

#include "preprocessor.h"

Error preprocessor_init(Preprocessor *preprocessor, const Lexer *lexer) {
    if (!lexer || !preprocessor) return ERROR_NULL_POINTER;

    preprocessor->files.stack_top = 1;
    preprocessor->files.stack_size = 8;
    preprocessor->files.sources = malloc(sizeof(Lexer) * 8);
    if (!preprocessor->files.sources) {
        return ERROR_ALLOCATION_FAILED;
    }

    preprocessor->files.sources[0] = *lexer;

    preprocessor->macro_table.count = 0;
    preprocessor->macro_table.capacity = 8;
    preprocessor->macro_table.entries = malloc(sizeof(Macro) * preprocessor->macro_table.capacity);
    if (!preprocessor->macro_table.entries) {
        free(preprocessor->files.sources);
        preprocessor->files.sources = nullptr;
        return ERROR_ALLOCATION_FAILED;
    }

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
