#ifndef NA_16_LEXER_H
#define NA_16_LEXER_H

#include <stdint.h>
#include <error.h>

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
    SourceLocation block_comment_start;
} Lexer;

typedef struct LexerError {
    SourceSpan span;
    char *message;
} LexerError;

void lexer_init(Lexer *lexer, const SourceFile *source);

/*
 * Returns true when a token was successfully produced.
 *
 * Lexical errors return false and populate error.
 */
ErrorCode lexer_next(Lexer *lexer, PPToken *token, LexerError *error);
ErrorCode lexer_next_header_name(Lexer *lexer, PPToken *token, LexerError *error, bool h_char);

#endif //NA_16_LEXER_H
