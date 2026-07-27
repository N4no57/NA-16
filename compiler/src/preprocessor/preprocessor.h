#ifndef NA_16_PREPROCESSOR_H
#define NA_16_PREPROCESSOR_H

#include "../lexer/lexer.h"
#include "../lexer/token.h"

typedef struct Preprocessor {
    Lexer *lexer;
} Preprocessor;

void preprocessor_init(Preprocessor *preprocessor, Lexer *lexer);

bool preprocessor_next(Preprocessor *preprocessor, PPToken *token, LexerError *error);

#endif //NA_16_PREPROCESSOR_H
