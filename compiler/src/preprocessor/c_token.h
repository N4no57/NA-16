#ifndef NA_16_C_TOKEN_H
#define NA_16_C_TOKEN_H

#include "../lexer/token.h"

typedef enum CTokenKind {
    C_TOKEN_EOF,

    C_TOKEN_IDENTIFIER,
    C_TOKEN_INTEGER_CONSTANT,

    C_TOKEN_KW_INT,
    C_TOKEN_KW_VOID,
    C_TOKEN_KW_RETURN,

    C_TOKEN_LEFT_PAREN,
    C_TOKEN_RIGHT_PAREN,
    C_TOKEN_LEFT_BRACE,
    C_TOKEN_RIGHT_BRACE,
    C_TOKEN_SEMICOLON
} CTokenKind;

typedef enum IntegerSuffixLength {
    INTEGER_SUFFIX_LENGTH_NONE,
    INTEGER_SUFFIX_LENGTH_LONG,
    INTEGER_SUFFIX_LENGTH_LONG_LONG
} IntegerSuffixLength;

typedef struct IntegerSuffix {
    IntegerSuffixLength length;
    bool is_unsigned;
} IntegerSuffix;

typedef struct CToken {
    CTokenKind kind;
    SourceSpan span;

    union {
        struct {
            union {
                uint64_t unsigned_int;
                int64_t signed_int;
            };
            IntegerSuffix suffix;
        } integer;

        struct {
            double floating;
        } floating_point;
    } data;
} CToken;

bool convert_ppt_to_ct(const PPToken *ppt, CToken *ct);

#endif //NA_16_C_TOKEN_H
