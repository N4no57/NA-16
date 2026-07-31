#ifndef NA_16_TOKEN_H
#define NA_16_TOKEN_H

#include <stddef.h>

#include "source.h"

typedef enum PPTokenKind {
    PP_TOKEN_EOF,
    PP_TOKEN_INVALID,

    PP_TOKEN_IDENTIFIER,
    PP_TOKEN_NUMBER,
    PP_TOKEN_CHARACTER_CONSTANT,
    PP_TOKEN_STRING_LITERAL,
    PP_TOKEN_HEADER_NAME,

    PP_TOKEN_LEFT_BRACKET,
    PP_TOKEN_RIGHT_BRACKET,
    PP_TOKEN_LEFT_PAREN,
    PP_TOKEN_RIGHT_PAREN,
    PP_TOKEN_LEFT_BRACE,
    PP_TOKEN_RIGHT_BRACE,

    PP_TOKEN_DOT,
    PP_TOKEN_ARROW,
    PP_TOKEN_INCREMENT,
    PP_TOKEN_DECREMENT,

    PP_TOKEN_AMPERSAND,
    PP_TOKEN_ASTERISK,
    PP_TOKEN_PLUS,
    PP_TOKEN_MINUS,
    PP_TOKEN_TILDE,
    PP_TOKEN_EXCLAMATION,
    PP_TOKEN_SLASH,
    PP_TOKEN_PERCENT,

    PP_TOKEN_SHIFT_LEFT,
    PP_TOKEN_SHIFT_RIGHT,

    PP_TOKEN_LESS,
    PP_TOKEN_GREATER,
    PP_TOKEN_LESS_EQUAL,
    PP_TOKEN_GREATER_EQUAL,
    PP_TOKEN_EQUAL_EQUAL,
    PP_TOKEN_NOT_EQUAL,

    PP_TOKEN_CARET,
    PP_TOKEN_PIPE,
    PP_TOKEN_LOGICAL_AND,
    PP_TOKEN_LOGICAL_OR,

    PP_TOKEN_QUESTION,
    PP_TOKEN_COLON,
    PP_TOKEN_SEMICOLON,
    PP_TOKEN_ELLIPSIS,

    PP_TOKEN_ASSIGN,
    PP_TOKEN_MULTIPLY_ASSIGN,
    PP_TOKEN_DIVIDE_ASSIGN,
    PP_TOKEN_REMAINDER_ASSIGN,
    PP_TOKEN_ADD_ASSIGN,
    PP_TOKEN_SUBTRACT_ASSIGN,
    PP_TOKEN_SHIFT_LEFT_ASSIGN,
    PP_TOKEN_SHIFT_RIGHT_ASSIGN,
    PP_TOKEN_AND_ASSIGN,
    PP_TOKEN_XOR_ASSIGN,
    PP_TOKEN_OR_ASSIGN,

    PP_TOKEN_COMMA,

    PP_TOKEN_HASH,
    PP_TOKEN_HASH_HASH,

    /*
     * A non-whitespace character that does not form any other
     * preprocessing token.
     */
    PP_TOKEN_OTHER_CHARACTER,

    /*
     * Internal token retained for directive handling.
     */
    PP_TOKEN_NEWLINE
} PPTokenKind;

typedef struct PPToken {
    PPTokenKind kind;
    SourceSpan actual_span;
    SourceSpan presumed_span;

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

    /*
     * Used when token is a string or character literal
     */
    bool wide;

    // token specific data
    union {
        char *string;
    } data;
} PPToken;

const char *pp_token_kind_name(PPTokenKind kind);

size_t pp_token_length(const PPToken *token);

#endif //NA_16_TOKEN_H
