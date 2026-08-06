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

    code = vector_init(&preprocessor->conditionals, sizeof(PPConditionalFrame));

    if (code != ERROR_OK) {
        vector_destroy(&preprocessor->macro_table);
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

                vector_set(&preprocessor->sources, preprocessor->sources.length-1, &source);

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

                *result = source.macro.tokens[source.macro.index++];
                vector_set(&preprocessor->sources, preprocessor->sources.length-1, &source);
                break;
            }
        }

        return ERROR_OK;
    }

    return ERROR_INTERNAL; // TODO error
}

static bool pp_active(Preprocessor *preprocessor) {
    if (preprocessor->conditionals.length > 0) {
        return ((PPConditionalFrame *)preprocessor->conditionals.data)[preprocessor->conditionals.length-1].branch_active;
    }
    return true;
}

static Error pp_peek_unexpanded(...) {

}

static Error pp_process_directive(Preprocessor *preprocessor, PreprocessorError *error) {
    Error code;
    PPToken token;
    PPSourceKind origin;

    if ((code = pp_read_unexpanded(preprocessor, &token, &origin, error)) != ERROR_OK) return code;

    if (origin != PP_FILE_SOURCE) { // TODO error
        return ERROR_INTERNAL;
    }

    if (token.kind == PP_TOKEN_NEWLINE) {
        return ERROR_OK;
    }

    if (token.kind != PP_TOKEN_IDENTIFIER) { // TODO error
        return ERROR_INTERNAL;
    }

    if (strcmp(token.data.string, "if") == 0 ||
        strcmp(token.data.string, "ifdef") == 0 ||
        strcmp(token.data.string, "ifndef") == 0
    ) {
        bool parent_active = pp_active(preprocessor);
        bool condition = false;


        PPConditionalFrame conditional = {
            .parent_active = parent_active,
            .branch_active = parent_active && condition,
            .branch_taken = parent_active && condition,
            .saw_else = false
        };

        vector_push(&preprocessor->conditionals, &conditional);
    } else if (strcmp(token.data.string, "elif") == 0) {

    } else if (strcmp(token.data.string, "else") == 0) {

    } else if (strcmp(token.data.string, "endif") == 0) {

    }

    if (!pp_active(preprocessor)) return ERROR_OK;

    if (strcmp(token.data.string, "define") == 0) {
        if ((code = pp_read_unexpanded(preprocessor, &token, &origin, error)) != ERROR_OK) return code;

        if (token.kind != PP_TOKEN_IDENTIFIER || origin != PP_FILE_SOURCE) { // TODO error
            return ERROR_INTERNAL;
        }

        Macro macro = {0};
        macro.name = strdup(token.data.string);

        if ((code = pp_read_unexpanded(preprocessor, &token, &origin, error)) != ERROR_OK) return code;

        // check if object like macro or function like macro
        if (token.kind == PP_TOKEN_LEFT_PAREN && !token.leading_space) {
            // function like
            macro.kind = MACRO_FUNCTION_LIKE;
        } else {
            // object like
            macro.kind = MACRO_OBJECT_LIKE;

            Vector tokens;
            vector_init(&tokens, sizeof(PPToken));

            while (token.kind != PP_TOKEN_NEWLINE) {
                if (origin != PP_FILE_SOURCE) { // TODO error
                    vector_destroy(&tokens);
                    free(macro.name);
                    return ERROR_INTERNAL;
                }

                vector_push(&tokens, &token);

                if ((code = pp_read_unexpanded(preprocessor, &token, &origin, error)) != ERROR_OK) return code;
            }

            macro.replacement_count = tokens.length;
            macro.replacement = (PPToken *)tokens.data;

            return macro_table_define(&preprocessor->macro_table, macro);
        }
    } else if (strcmp(token.data.string, "undef") == 0) {
        if ((code = pp_read_unexpanded(preprocessor, &token, &origin, error)) != ERROR_OK) return code;

        if (token.kind != PP_TOKEN_IDENTIFIER || origin != PP_FILE_SOURCE) { // TODO error
            return ERROR_INTERNAL;
        }

        macro_table_undef(&preprocessor->macro_table, token.data.string);
        return ERROR_OK;
    } else if (strcmp(token.data.string, "include") == 0) {

    } else if (strcmp(token.data.string, "line") == 0) {

    } else if (strcmp(token.data.string, "error") == 0) {

    } else if (strcmp(token.data.string, "pragma") == 0) {

    }
}

static Error pp_try_expand_macro(Preprocessor *preprocessor, PPToken *identifier, bool *expanded) {
    Macro *macro = macro_table_find(&preprocessor->macro_table, identifier->data.string);

    if (macro == nullptr) {
        *expanded = false;
        return ERROR_OK;
    }

    if (macro->kind == MACRO_OBJECT_LIKE) {
        PPSourceFrame expansion = {0};

        expansion.kind = PP_MACRO_SOURCE;
        expansion.macro.macro = macro;
        expansion.macro.count = macro->replacement_count;
        expansion.macro.tokens = macro->replacement;

        vector_push(&preprocessor->sources, &expansion);
    } else if (macro->kind == MACRO_FUNCTION_LIKE) {

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

        if (!pp_active(preprocessor)) {
            continue;
        }

        if (token.kind == PP_TOKEN_NEWLINE) {
            continue;
        }

        if (token.kind == PP_TOKEN_IDENTIFIER) {
            bool expanded;

            pp_try_expand_macro(preprocessor, &token, &expanded);

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
