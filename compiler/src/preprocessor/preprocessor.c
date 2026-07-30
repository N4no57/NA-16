#include <stdlib.h>
#include <string.h>

#include "preprocessor.h"

#include <stdio.h>
#include <wchar.h>

static char *(directives[]) = {
    "if",
    "ifdef",
    "ifndef",
    "elif",
    "else",
    "endif",
    "include",
    "define",
    "undef",
    "line",
    "error",
    "pragma"
};

static bool is_directive(const PPToken *token) {
    if (token->kind != PP_TOKEN_IDENTIFIER) {
        return false;
    }

    const char *identifier = token->data.string;

    for (size_t i = 0; i < _countof(directives); i++) {
        if (strcmp(directives[i], identifier) == 0) {
            return true;
        }
    }

    return false;
}

static bool push_source_stack(Preprocessor *preprocessor, const PPTokenSource *item) {
    if (preprocessor->stack_top >= preprocessor->stack_size) {
        preprocessor->stack_size *= 2;
        PPTokenSource *tmp = realloc(preprocessor->sources, preprocessor->stack_size * sizeof(PPTokenSource));
        if (!tmp) {
            return false;
        }
        preprocessor->sources = tmp;
    }

    preprocessor->sources[preprocessor->stack_top++] = *item;
    return true;
}

void preprocessor_init(Preprocessor *preprocessor, const Lexer *lexer) {
    preprocessor->stack_top = 1;
    preprocessor->stack_size = 8;
    preprocessor->sources = malloc(sizeof(PPTokenSource) * 8);
    preprocessor->sources[0] = (PPTokenSource){
        .kind = PP_SOURCE_FILE,
        .file = {
            *lexer
        }
    };

    preprocessor->macro_table.count = 0;
    preprocessor->macro_table.capacity = 8;
    preprocessor->macro_table.entries = malloc(sizeof(Macro) * preprocessor->macro_table.capacity);
}

static bool get_next_token(Preprocessor *preprocessor, PPToken *token, LexerError *error) {
    if (preprocessor->stack_top == 0) return false;

    PPTokenSource *source = &preprocessor->sources[preprocessor->stack_top-1];

    switch (source->kind) {
        case PP_SOURCE_FILE:
            if (!lexer_next(&source->file.lexer, token, error)) return false;

            if (token->kind == PP_TOKEN_EOF) {
                preprocessor->stack_top--;
                if (preprocessor->stack_top == 0) return true; // ran out of tokens so return EOF

                while (token->kind == PP_TOKEN_EOF) { // otherwise search for a token that isn't EOF
                    if (!get_next_token(preprocessor, token, error)) {
                        return false;
                    }
                }
            }

            return true;
        case PP_SOURCE_MACRO_EXPANSION:
            *token = source->expansion.tokens[source->expansion.index++];

            if (source->expansion.index >= source->expansion.count) {
                preprocessor->stack_top--;
                free(source->expansion.tokens);
            }

            return true;
    }

    return false;
}

static bool get_parameters(Preprocessor *preprocessor, PPToken *token, LexerError *error, Macro *macro) {
    macro->kind = MACRO_FUNCTION_LIKE;
    if (!get_next_token(preprocessor, token, error)) return false;

    char **parameter_list = malloc(sizeof(char *) * 8);
    size_t capacity = 8;
    size_t count = 0;


    while (token->kind != PP_TOKEN_EOF &&
        token->kind != PP_TOKEN_RIGHT_PAREN
    ) {
        if (count >= capacity) {
            capacity *= 2;
            char **tmp = realloc(parameter_list, sizeof(char *) * capacity);
            if (!tmp) {
                return false;
            }
            parameter_list = tmp;
        }

        if (macro->variadic) return false;

        if (token->kind == PP_TOKEN_ELLIPSIS) macro->variadic = true;
        else if (token->kind != PP_TOKEN_IDENTIFIER) return false;

        if (!macro->variadic) parameter_list[count++] = token->data.string;

        if (!get_next_token(preprocessor, token, error)) return false;
        if (token->kind != PP_TOKEN_COMMA) break;
        if (!get_next_token(preprocessor, token, error)) return false;
    }

    if (token->kind != PP_TOKEN_RIGHT_PAREN) return false;
    if (!get_next_token(preprocessor, token, error)) return false;

    macro->parameter_count = count;
    macro->parameters = parameter_list;

    return true;
}

static PPToken *get_replacement_list(Preprocessor *preprocessor, PPToken *token, LexerError *error, size_t *count) {
    PPToken *token_list = malloc(sizeof(PPToken) * 8);
    size_t capacity = 8;
    *count = 0;

    while (token->kind != PP_TOKEN_EOF &&
        token->kind != PP_TOKEN_NEWLINE
    ) {
        if (*count >= capacity) {
            capacity *= 2;
            PPToken *tmp = realloc(token_list, capacity * sizeof(PPToken));
            if (!tmp) {
                free(token_list);
                return nullptr;
            }
            token_list = tmp;
        }

        token_list[(*count)++] = *token;
        if (!get_next_token(preprocessor, token, error)) return nullptr;
    }

    return token_list;
}

static bool parse_directive(Preprocessor *preprocessor, PPToken *token, LexerError *error) {
    char *directive = token->data.string;
    const SourceLocation start = token->span.begin;
    if (!get_next_token(preprocessor, token, error)) return false;

    if (strcmp("define", directive) == 0) {
        if (token->kind != PP_TOKEN_IDENTIFIER) {
            return false;
        }

        Macro macro = {
            .name = copy_string(&token->span),
            .kind = MACRO_OBJECT_LIKE,
            .definition_span = {
                .begin = start
            }
        };

        if (!get_next_token(preprocessor, token, error)) return false;

        if (token->kind == PP_TOKEN_LEFT_PAREN && !token->leading_space) {
            // it is a function like macro and should consume argument list
            get_parameters(preprocessor, token, error, &macro);
        }

        macro.replacement = get_replacement_list(preprocessor, token, error, &macro.replacement_count);
        macro.definition_span.end = macro.replacement[macro.replacement_count-1].span.end;

        macro_table_define(&preprocessor->macro_table, macro);
    } else if (strcmp("undef", directive) == 0) {
        if (token->kind != PP_TOKEN_IDENTIFIER) {
            free(directive);
            return false;
        }
        const char *name = token->data.string;

        macro_table_undef(&preprocessor->macro_table, name);
    }

    return true;
}

typedef struct {
    size_t begin;
    size_t end;
} Argument;

typedef struct {
    Argument *arguments;
    size_t count;
    size_t capacity;
} ArgList;

typedef struct {
    PPToken *tokens;
    size_t count;
    size_t capacity;
} PPTokenBuffer;

static bool push_token(PPTokenBuffer *buf, const PPToken *token) {
    if (buf->count >= buf->capacity) {
        buf->capacity *= 2;
        PPToken *tmp = realloc(buf->tokens, buf->capacity * sizeof(PPToken));
        if (!tmp) {
            free(buf->tokens);
            return false;
        }
        buf->tokens = tmp;
    }

    buf->tokens[buf->count++] = *token;
    return true;
}

static bool push_argument(ArgList *arg_list, const Argument *arg) {
    if (arg_list->count >= arg_list->capacity) {
        arg_list->capacity *= 2;
        Argument *tmp = realloc(arg_list->arguments, arg_list->capacity * sizeof(Argument));
        if (!tmp) {
            free(arg_list->arguments);
            return false;
        }
        arg_list->arguments = tmp;
    }

    arg_list->arguments[arg_list->count++] = *arg;
    return true;
}

static bool collect_macro_arguments(
    Preprocessor *preprocessor,
    PPToken *token,
    LexerError *error,
    const Macro *macro,
    PPTokenBuffer *token_buf,
    ArgList *arg_list
) {
    if (token->kind != PP_TOKEN_LEFT_PAREN) return false;
    if (!get_next_token(preprocessor, token, error)) return false;

    size_t depth = 0;
    size_t params = 0;
    bool is_empty = true;

    arg_list->count = 0;
    arg_list->capacity = macro->parameter_count * 2;
    arg_list->arguments = malloc(sizeof(Argument) * arg_list->capacity);
    arg_list->arguments[0].begin = 0;

    Argument current_arg = {0};

    token_buf->count = 0;
    token_buf->capacity = 8;
    token_buf->tokens = malloc(sizeof(PPToken) * token_buf->capacity);

    while (true) {
        if (token->kind == PP_TOKEN_RIGHT_PAREN && depth == 0) break;
        if (token->kind == PP_TOKEN_RIGHT_PAREN && depth > 0) depth--;
        if (token->kind == PP_TOKEN_LEFT_PAREN) depth++;
        if (token->kind == PP_TOKEN_COMMA && depth == 0) {
            params++;
            if (!push_argument(arg_list, &current_arg)) {
                free(token_buf->tokens);
                return false;
            }

            current_arg.begin = token_buf->count;
            if (!get_next_token(preprocessor, token, error)) return false;
            continue;
        }

        is_empty = false;

        if (!push_token(token_buf, token)) {
            free(arg_list->arguments);
            return false;
        }

        current_arg.end = token_buf->count;

        if (!get_next_token(preprocessor, token, error)) return false;
    }

    current_arg.end = token_buf->count;
    if (!push_argument(arg_list, &current_arg)) return false;
    if (!is_empty) params++;

    if (macro->variadic && params < macro->parameter_count) return false;
    if (!macro->variadic && params != macro->parameter_count) return false;

    return true;
}

static bool match_parameter(const Macro *macro, const char *string, size_t *idx) {
    for (size_t i = 0; i < macro->parameter_count; i++) {
        if (strcmp(macro->parameters[i], string) == 0) {
            if (idx != nullptr) *idx = i;
            return true;
        }
    }

    return false;
}

static PPToken *generate_expansion(const Macro *macro, const ArgList *args, const PPTokenBuffer *arg_list) {
    const Argument va_args = {
        .begin = macro->parameter_count,
        .end = args->count
    };

    PPTokenBuffer token_list = {
        .count = 0,
        .capacity = macro->replacement_count * 2
    };
    token_list.tokens = malloc(sizeof(PPToken) * token_list.capacity);

    for (size_t i = 0; i < macro->replacement_count; i++) {
        const PPToken *token = &macro->replacement[i];

        if (token->kind == PP_TOKEN_HASH) {
            // stringify
            if (i++ >= macro->replacement_count) return nullptr;
            const PPToken *next = &macro->replacement[i];
            if (next->kind != PP_TOKEN_IDENTIFIER) return nullptr;

            const char *identifier = next->data.string;

            size_t idx;

            if (match_parameter(macro, identifier, &idx)) {
                size_t size = 0;
                char *stringification_str = nullptr;
                for (size_t j = args->arguments[idx].begin; j < args->arguments[idx].end; j++) {
                    char *str = copy_string(&arg_list->tokens[j].span);
                    size_t str_size = strlen(str);
                    if (arg_list->tokens[j].leading_space) str_size++;

                    char *tmp = realloc(stringification_str, size + str_size + 1);
                    if (!tmp) {
                        free(stringification_str);
                        free(str);
                        return nullptr;
                    }
                    stringification_str = tmp;

                    if (arg_list->tokens[j].leading_space) {
                        stringification_str[size] = ' ';
                        size++;
                        str_size--;
                    }

                    memcpy(&stringification_str[size], str, str_size);
                    size += str_size;

                    free(str);
                }

                stringification_str[size] = '\0';

                PPToken token_to_push = {
                    .kind = PP_TOKEN_STRING_LITERAL,
                    .leading_space = false,
                    .start_of_line = false,
                    .span = {
                        .begin = token->span.begin,
                        .end = arg_list->tokens[args->arguments[idx].end].span.end
                    }
                };

                push_token(&token_list, &token_to_push);

                free(stringification_str);
            }
        }

        if (token->kind == PP_TOKEN_HASH_HASH) {
            // combinify
        }

        if (token->kind == PP_TOKEN_IDENTIFIER) {
            const char *identifier = token->data.string;
            if (identifier == nullptr) return nullptr;

            size_t idx;

            if (match_parameter(macro, identifier, &idx)) {
                for (size_t j = args->arguments[idx].begin; j < args->arguments[idx].end; j++) {
                    push_token(&token_list, &arg_list->tokens[j]);
                }

                continue;
            }

            if (strcmp("__VA_ARGS__", identifier) == 0) {
                for (size_t j = va_args.begin; j < va_args.end; j++) {
                    push_token(&token_list, &(PPToken){
                        .kind = PP_TOKEN_COMMA,
                        .leading_space = false,
                        .start_of_line = false,
                        .wide = false
                    });

                    for (size_t k = args->arguments[j].begin; k < args->arguments[j].end; k++) {
                        push_token(&token_list, &arg_list->tokens[k]);
                    }
                }

                continue;
            }
        }

        push_token(&token_list, token);
    }

    return token_list.tokens;
}

typedef enum {
    IDENTIFIER_RESULT_NONE,
    IDENTIFIER_RESULT_OKAY,
    IDENTIFIER_RESULT_ERROR
} IdentifierResult;

static IdentifierResult process_identifier(Preprocessor *preprocessor, PPToken *token, LexerError *error) {
    const char *identifier = token->data.string;
    if (identifier == nullptr) return IDENTIFIER_RESULT_ERROR;

    Macro *macro = macro_table_find(&preprocessor->macro_table, identifier);

    if (macro != nullptr) {
        if (!get_next_token(preprocessor, token, error)) return IDENTIFIER_RESULT_ERROR;

        PPTokenBuffer buff;
        ArgList arg_list;

        if (macro->kind == MACRO_FUNCTION_LIKE && macro->parameter_count > 0) {
            if (!collect_macro_arguments(
                preprocessor,
                token,
                error,
                macro,
                &buff,
                &arg_list
            )) return IDENTIFIER_RESULT_ERROR;
        }

        push_source_stack(
            preprocessor,
            &(PPTokenSource){
                .kind = PP_SOURCE_MACRO_EXPANSION,
                .expansion = {
                    .count = macro->replacement_count,
                    .index = 0,
                    .macro = macro,
                    .tokens = generate_expansion(macro, &arg_list, &buff)
                }
            }
        );

        free(buff.tokens);
        free(arg_list.arguments);

        return IDENTIFIER_RESULT_OKAY;
    }

    return IDENTIFIER_RESULT_NONE;
}

bool preprocessor_next(Preprocessor *preprocessor, PPToken *token, LexerError *error) {
    if (!preprocessor || !preprocessor->sources || !token || preprocessor->stack_top == 0) {
        return false;
    }

    for (;;) {
        if (!get_next_token(preprocessor, token, error)) {
            return false;
        }

        if (token->kind == PP_TOKEN_HASH) {
            if (!get_next_token(preprocessor, token, error)) return false;
            if (!is_directive(token)) continue;

            parse_directive(preprocessor, token, error);

            continue;
        }

        if (token->kind == PP_TOKEN_IDENTIFIER) {
            const IdentifierResult result = process_identifier(preprocessor, token, error);
            if (result == IDENTIFIER_RESULT_ERROR) return false;
            if (result == IDENTIFIER_RESULT_OKAY) continue;
        }

        if (token->kind == PP_TOKEN_NEWLINE) {
            continue;
        }

        return true;
    }
}

void preprocessor_destroy(const Preprocessor *preprocessor) {
    for (size_t i = 0; i < preprocessor->macro_table.count; i++) {
        const Macro *entry = &preprocessor->macro_table.entries[i];
        free(entry->name);
        if (entry->parameter_count > 0) {
            for (size_t j = 0; j < entry->parameter_count; j++) {
                free(entry->parameters[j]);
            }
            free(entry->parameters);
        }

        if (entry->replacement_count > 0) {
            free(entry->replacement);
        }
    }

    free(preprocessor->macro_table.entries);
    free(preprocessor->sources);
}
