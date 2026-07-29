#include <stdlib.h>
#include <string.h>

#include "preprocessor.h"

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

#define countof(x) (sizeof(x)/sizeof((x)[0]))

static char *copy_string(const SourceSpan *span) {
    const size_t size = span->end.column - span->begin.column;
    char *str = malloc(size+1);

    if (!str) {
        return nullptr;
    }

    const char *start = &span->begin.file->contents[span->begin.offset];
    memcpy(str, start, size);
    str[size] = '\0';

    return str;
}

static bool is_directive(const PPToken *token) {
    if (token->kind != PP_TOKEN_IDENTIFIER) {
        return false;
    }

    char *identifier = copy_string(&token->span);

    for (size_t i = 0; i < countof(directives); i++) {
        if (strcmp(directives[i], identifier) == 0) {
            free(identifier);
            return true;
        }
    }

    free(identifier);
    return false;
}

static bool push_stack(Preprocessor *preprocessor, PPToken *token) {
    if (preprocessor->stack_top >= preprocessor->stack_size) {
        preprocessor->stack_size *= 2;
        PPToken *tmp = realloc(preprocessor->stack, preprocessor->stack_size * sizeof(PPToken));
        if (!tmp) {
            return false;
        }
        preprocessor->stack = tmp;
    }

    preprocessor->stack[preprocessor->stack_top++] = *token;
    return true;
}

static PPToken pop_stack(Preprocessor *preprocessor) {
    return preprocessor->stack[--preprocessor->stack_top];
}

void preprocessor_init(Preprocessor *preprocessor, Lexer *lexer) {
    preprocessor->lexer = lexer;

    preprocessor->macro_table.count = 0;
    preprocessor->macro_table.capacity = 8;
    preprocessor->macro_table.entries = malloc(sizeof(Macro) * preprocessor->macro_table.capacity);

    preprocessor->stack_top = 0;
    preprocessor->stack_size = 8;
    preprocessor->stack = malloc(sizeof(PPToken) * preprocessor->stack_size);
}

static bool get_next_token(Preprocessor *preprocessor, PPToken *token, LexerError *error) {
    if (preprocessor->stack_top > 0) {
        *token = pop_stack(preprocessor);
        return true;
    }

    return lexer_next(preprocessor->lexer, token, error);
}

static bool parse_directive(Preprocessor *preprocessor, PPToken *token, LexerError *error) {
    char *directive = copy_string(&token->span);
    const SourceLocation start = token->span.begin;
    if (!get_next_token(preprocessor, token, error)) return false;

    if (strcmp("define", directive) == 0) {
        if (token->kind != PP_TOKEN_IDENTIFIER) {
            free(directive);
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

        PPToken *token_list = malloc(sizeof(PPToken) * 8);
        size_t capacity = 8;
        size_t count = 0;

        if (token->kind == PP_TOKEN_LEFT_PAREN && !token->leading_space) {
            // it is a function like macro and should consume argument list
            macro.kind = MACRO_FUNCTION_LIKE;
            if (!get_next_token(preprocessor, token, error)) return false;

            while (token->kind != PP_TOKEN_EOF &&
                token->kind != PP_TOKEN_RIGHT_PAREN
            ) {
                if (count >= capacity) {
                    capacity *= 2;
                    PPToken *tmp = realloc(preprocessor->stack, capacity * sizeof(PPToken));
                    if (!tmp) {
                        return false;
                    }
                    token_list = tmp;
                }

                if (token->kind != PP_TOKEN_IDENTIFIER) return false;

                token_list[count++] = *token;

                if (!get_next_token(preprocessor, token, error)) return false;
                if (token->kind != PP_TOKEN_COMMA) break;
                if (!get_next_token(preprocessor, token, error)) return false;
            }

            if (token->kind != PP_TOKEN_RIGHT_PAREN) return false;
            if (!get_next_token(preprocessor, token, error)) return false;

            macro.parameter_count = count;
            macro.parameters = malloc(sizeof(char *) * macro.parameter_count);

            for (size_t i = 0; i < macro.parameter_count; i++) {
                macro.parameters[i] = copy_string(&token->span);
            }

            count = 0;
        }

        while (token->kind != PP_TOKEN_EOF &&
            token->kind != PP_TOKEN_NEWLINE
        ) {
            if (count >= capacity) {
                capacity *= 2;
                PPToken *tmp = realloc(token_list, capacity * sizeof(PPToken));
                if (!tmp) {
                    free(directive);
                    free(token_list);
                    return false;
                }
                token_list = tmp;
            }

            token_list[count++] = *token;
            get_next_token(preprocessor, token, nullptr);
        }

        macro.replacement_count = count;
        macro.replacement = malloc(sizeof(PPToken) * macro.replacement_count);
        memcpy(macro.replacement, token_list, sizeof(PPToken) * macro.replacement_count);
        macro.definition_span.end = macro.replacement[macro.replacement_count-1].span.end;
        free(token_list);

        macro_table_define(&preprocessor->macro_table, macro);
    } else if (strcmp("undef", directive) == 0) {
        if (token->kind != PP_TOKEN_IDENTIFIER) {
            free(directive);
            return false;
        }
        char *name = copy_string(&token->span);

        macro_table_undef(&preprocessor->macro_table, name);
        free(name);
    }

    free(directive);
    return true;
}

bool preprocessor_next(Preprocessor *preprocessor, PPToken *token, LexerError *error) {
    if (!preprocessor || !preprocessor->lexer || !token) {
        return false;
    }

    for (;;) {
        if (!get_next_token(preprocessor, token, error)) {
            return false;
        }

        if (token->kind == PP_TOKEN_HASH) {
            get_next_token(preprocessor, token, error);
            if (!is_directive(token)) continue;

            parse_directive(preprocessor, token, error);

            continue;
        }

        if (token->kind == PP_TOKEN_IDENTIFIER) {
            char *identifier = copy_string(&token->span);

            Macro *macro = macro_table_find(&preprocessor->macro_table, identifier);

            if (macro != nullptr) {
                for (size_t i = macro->replacement_count; i-- > 0;) {
                    push_stack(preprocessor, macro->replacement);
                }

                free(identifier);
                continue;
            }

            free(identifier);
        }

        if (token->kind == PP_TOKEN_NEWLINE) {
            continue;
        }

        return true;
    }
}

void preprocessor_destroy(const Preprocessor *preprocessor) {
    free(preprocessor->macro_table.entries);
    free(preprocessor->stack);
}
