#include <stdlib.h>
#include <error.h>
#include <string.h>

#include "preprocessor.h"

Error preprocessor_init(Preprocessor *preprocessor, const Lexer *lexer) {
    if (!lexer || !preprocessor) return ERROR_NULL_POINTER;

    Error code = vector_init(&preprocessor->sources, sizeof(PPSourceFrame));
    if (code != ERROR_OK) {
        return code;
    }

    code = vector_init(&preprocessor->macro_table, sizeof(Macro));

    if (code != ERROR_OK) {
        vector_destroy(&preprocessor->sources);
        return code;
    }

    const PPSourceFrame source = {
        .kind = PP_FILE_SOURCE,
        .file = *lexer
    };

    vector_push(&preprocessor->sources, &source);

    return ERROR_OK;
}

static void generate_error(PreprocessorError *error, SourceSpan error_location,  char *message) {
    if (error == nullptr) return;

    *error = (PreprocessorError){
        .message = strdup(message),
        .span = error_location
    };
}

static Error pp_read_unexpanded(Preprocessor *preprocessor, PPToken *result, PPSourceKind *origin, PreprocessorError *error) {
    while (preprocessor->sources.length > 0) {
        PPSourceFrame source;
        Error code;
        if ((code = vector_get(&preprocessor->sources, preprocessor->sources.length-1, &source)) != ERROR_OK) {
            return code;
        }
        *origin = source.kind;


        switch (source.kind) {
            case PP_FILE_SOURCE: {
                PPToken token;

                if ((code = lexer_next(&source.file, &token, error)) != ERROR_OK) {
                    return code;
                }

                if (token.kind != PP_TOKEN_EOF) {
                    *result = token;
                    return ERROR_OK;
                }

                // EOF from a header file
                if (preprocessor->sources.length > 1) {
                    if ((code = vector_pop(&preprocessor->sources, nullptr))) return code;
                    continue;
                }

                // EOF from the source file which means: ITS TIME TO STOP
                *result = token;
                break;
            }
            case PP_MACRO_SOURCE: {
                if (source.macro.index >= source.macro.count) {
                    if (source.macro.owns_tokens) free(source.macro.tokens);

                    if ((code = vector_pop(&preprocessor->sources, nullptr))) return code;
                    continue;
                }

                *result = source.macro.tokens[source.macro.index];
                break;
            }
        }

        return ERROR_OK;
    }

    return ERROR_INTERNAL;
}

static Error pp_peek_unexpanded(...);

static Error pp_process_directive(Preprocessor *preprocessor, PreprocessorError *error) {
    Error code;
    PPToken token;
    PPSourceKind origin;

    if ((code = pp_read_unexpanded(preprocessor, &token, &origin, error)) != ERROR_OK) {
        return code;
    }

    if (token.kind != PP_TOKEN_IDENTIFIER || origin != PP_FILE_SOURCE) {
        return ERROR_INTERNAL;
    }
}

static Error pp_try_expand_macro(Preprocessor *preprocessor, PPToken *identifier, bool *expanded) {
    Macro *macro = macro_table_find(&preprocessor->macro_table, identifier->data.string);

    if (macro == nullptr) {
        *expanded = false;
        return ERROR_OK;
    }

    *expanded = true;
    return ERROR_OK;
}

static Error pp_collect_arguments(...);
static Error pp_generate_replacement(...);
static Error pp_push_source(...);
static Error pp_is_active(...);

Error preprocessor_next(Preprocessor *preprocessor, PPToken *result, PreprocessorError *error) {
    if (!preprocessor || !result) {
        return ERROR_NULL_POINTER;
    }

    for (;;) {
        Error code;
        PPToken token;
        PPSourceKind source;

        if ((code = pp_read_unexpanded(preprocessor, &token, &source, error)) != ERROR_OK) {
            return code;
        }

        if (token.kind == PP_TOKEN_EOF) {
            *result = token;
            return ERROR_OK;
        }

        // only raw files can create preprocessing directives
        if (source == PP_FILE_SOURCE && token.kind == PP_TOKEN_HASH && token.start_of_line) {
            if ((code = pp_process_directive(preprocessor, error)) != ERROR_OK) {
                return code;
            }

            continue;
        }

        if (token.kind == PP_TOKEN_NEWLINE) {
            continue;
        }

        if (token.kind == PP_TOKEN_IDENTIFIER) {
            bool expanded;

            if ((code = pp_try_expand_macro(preprocessor, &token, &expanded)) != ERROR_OK) {
                return code;
            }

            if (expanded) continue;
        }

        *result = token;
        return ERROR_OK;
    }
}

void preprocessor_destroy(Preprocessor *preprocessor) {
    vector_destroy(&preprocessor->sources);

    // TODO destroy macro table

    vector_destroy(&preprocessor->macro_table);
}
