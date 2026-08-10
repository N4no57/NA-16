#ifndef NA_16_C_TOKEN_H
#define NA_16_C_TOKEN_H

#include "../lexer/token.h"

typedef enum CTokenKind {
    C_TOKEN_EOF,

    C_TOKEN_IDENTIFIER,
    C_TOKEN_INTEGER_CONSTANT,
    C_TOKEN_FLOATING_CONSTANT,
    C_TOKEN_CHARACTER_CONSTANT,
    C_TOKEN_STRING_LITERAL,

    C_TOKEN_KW_AUTO,
    C_TOKEN_KW_BREAK,
    C_TOKEN_KW_CASE,
    C_TOKEN_KW_CHAR,
    C_TOKEN_KW_CONST,
    C_TOKEN_KW_CONTINUE,
    C_TOKEN_KW_DEFAULT,
    C_TOKEN_KW_DO,
    C_TOKEN_KW_DOUBLE,
    C_TOKEN_KW_ELSE,
    C_TOKEN_KW_ENUM,
    C_TOKEN_KW_EXTERN,
    C_TOKEN_KW_FLOAT,
    C_TOKEN_KW_FOR,
    C_TOKEN_KW_GOTO,
    C_TOKEN_KW_IF,
    C_TOKEN_KW_INLINE,
    C_TOKEN_KW_INT,
    C_TOKEN_KW_LONG,
    C_TOKEN_KW_REGISTER,
    C_TOKEN_KW_RESTRICT,
    C_TOKEN_KW_RETURN,
    C_TOKEN_KW_SHORT,
    C_TOKEN_KW_SIGNED,
    C_TOKEN_KW_SIZEOF,
    C_TOKEN_KW_STATIC,
    C_TOKEN_KW_STRUCT,
    C_TOKEN_KW_SWITCH,
    C_TOKEN_KW_TYPEDEF,
    C_TOKEN_KW_UNION,
    C_TOKEN_KW_UNSIGNED,
    C_TOKEN_KW_VOID,
    C_TOKEN_KW_VOLATILE,
    C_TOKEN_KW_WHILE,
    C_TOKEN_KW__BOOL,
    C_TOKEN_KW__COMPLEX,
    C_TOKEN_KW__IMAGINARY,

    C_TOKEN_LEFT_BRACKET,
    C_TOKEN_RIGHT_BRACKET,
    C_TOKEN_LEFT_PAREN,
    C_TOKEN_RIGHT_PAREN,
    C_TOKEN_LEFT_BRACE,
    C_TOKEN_RIGHT_BRACE,

    C_TOKEN_DOT,
    C_TOKEN_ARROW,
    C_TOKEN_INCREMENT,
    C_TOKEN_DECREMENT,

    C_TOKEN_AMPERSAND,
    C_TOKEN_ASTERISK,
    C_TOKEN_PLUS,
    C_TOKEN_MINUS,
    C_TOKEN_TILDE,
    C_TOKEN_EXCLAMATION,
    C_TOKEN_SLASH,
    C_TOKEN_PERCENT,

    C_TOKEN_SHIFT_LEFT,
    C_TOKEN_SHIFT_RIGHT,

    C_TOKEN_LESS,
    C_TOKEN_GREATER,
    C_TOKEN_LESS_EQUAL,
    C_TOKEN_GREATER_EQUAL,
    C_TOKEN_EQUAL_EQUAL,
    C_TOKEN_NOT_EQUAL,

    C_TOKEN_CARET,
    C_TOKEN_PIPE,
    C_TOKEN_LOGICAL_AND,
    C_TOKEN_LOGICAL_OR,

    C_TOKEN_QUESTION,
    C_TOKEN_COLON,
    C_TOKEN_SEMICOLON,
    C_TOKEN_ELLIPSIS,

    C_TOKEN_ASSIGN,
    C_TOKEN_MULTIPLY_ASSIGN,
    C_TOKEN_DIVIDE_ASSIGN,
    C_TOKEN_REMAINDER_ASSIGN,
    C_TOKEN_ADD_ASSIGN,
    C_TOKEN_SUBTRACT_ASSIGN,
    C_TOKEN_SHIFT_LEFT_ASSIGN,
    C_TOKEN_SHIFT_RIGHT_ASSIGN,
    C_TOKEN_AND_ASSIGN,
    C_TOKEN_XOR_ASSIGN,
    C_TOKEN_OR_ASSIGN,

    C_TOKEN_COMMA,

    C_TOKEN_COUNT
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

        struct {
            char *str;
        } string_or_character;
    } data;
} CToken;

Error convert_ppt_to_ct(const PPToken *ppt, CToken *ct);

#endif //NA_16_C_TOKEN_H
