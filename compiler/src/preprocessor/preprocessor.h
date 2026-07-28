#ifndef NA_16_PREPROCESSOR_H
#define NA_16_PREPROCESSOR_H

#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include "macro.h"

typedef struct Preprocessor {
    Lexer *lexer;

    MacroTable macro_table;

    PPToken *stack;
    size_t stack_size;
    size_t stack_top;
} Preprocessor;

void preprocessor_init(Preprocessor *preprocessor, Lexer *lexer);

bool preprocessor_next(Preprocessor *preprocessor, PPToken *token, LexerError *error);

void preprocessor_destroy(const Preprocessor *preprocessor);

#endif //NA_16_PREPROCESSOR_H
