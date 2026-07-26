#ifndef NA_16_LEXER_H
#define NA_16_LEXER_H

#include <stdint.h>

#include "source.h"
#include "token.h"

typedef struct Lexer {
    const SourceFile *source;

    size_t offset;
    uint32_t line;
    uint32_t column;

    bool start_of_line;
    bool pending_space;

    bool inside_block_comment;
} Lexer;

typedef struct LexerError {
    SourceSpan span;
    const char *message;
} LexerError;

void lexer_init(Lexer *lexer, const SourceFile *source);

/*
 * Returns true when a token was successfully produced.
 *
 * Lexical errors return false and populate error.
 */
bool lexer_next(Lexer *lexer, PPToken *token, LexerError *error);

#endif //NA_16_LEXER_H
