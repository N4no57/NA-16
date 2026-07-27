#ifndef NA_16_TOKEN_H
#define NA_16_TOKEN_H

#include <stddef.h>

#include "source.h"

typedef enum PPTokenKind {
    PP_TOKEN_EOF,
    PP_TOKEN_INVALID,

    PP_TOKEN_IDENTIFIER,
    PP_TOKEN_NUMBER,

    PP_TOKEN_LEFT_PAREN,
    PP_TOKEN_RIGHT_PAREN,
    PP_TOKEN_LEFT_BRACE,
    PP_TOKEN_RIGHT_BRACE,
    PP_TOKEN_SEMICOLON,
    PP_TOKEN_COMMA,

    PP_TOKEN_NEWLINE
} PPTokenKind;

typedef struct PPToken {
    PPTokenKind kind;
    SourceSpan span;

    /*
     * True when whitespace or a comment appeared immediately before
     * this token.
     *
     * The preprocessor will eventually require this information when
     * reproducing tokens, handling stringification, and recognising
     * directives.
     */
    bool leading_space;

    /*
     * True when this is the first non-whitespace token on its logical
     * source line.
     *
     * This will eventually distinguish a preprocessing directive:
     *
     *     #define VALUE 42
     *
     * from an ordinary # token in another context.
     */
    bool start_of_line;
} PPToken;

const char *pp_token_kind_name(PPTokenKind kind);

size_t pp_token_length(const PPToken *token);

#endif //NA_16_TOKEN_H
