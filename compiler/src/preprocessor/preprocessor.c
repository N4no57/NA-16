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

bool preprocessor_next(Preprocessor *preprocessor, PPToken *token, LexerError *error) {
    if (!preprocessor || !preprocessor->lexer || !token) {
        return false;
    }

    for (;;) {
        if (preprocessor->stack_top > 0) {
            *token = pop_stack(preprocessor);
            return true;
        }

        if (!lexer_next(preprocessor->lexer, token, error)) {
            return false;
        }

        if (token->kind == PP_TOKEN_HASH) {
            lexer_next(preprocessor->lexer, token, error);
            is_directive(token);
            continue;
        }

        if (token->kind == PP_TOKEN_IDENTIFIER) {
            char *identifier = copy_string(&token->span);

            Macro *macro = macro_table_find(&preprocessor->macro_table, identifier);

            if (macro != nullptr) {

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
