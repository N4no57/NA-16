#ifndef NA_16_PREPROCESSOR_H
#define NA_16_PREPROCESSOR_H

#include <error.h>

#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include "macro.h"

typedef struct PPFileStack {
    Lexer *sources;
    size_t stack_size;
    size_t stack_top;
} PPFileStack;

typedef LexerError PreprocessorError;

typedef struct Preprocessor {
    PPFileStack files;
    MacroTable macro_table;
} Preprocessor;

Error preprocessor_init(Preprocessor *preprocessor, const Lexer *lexer);

Error preprocessor_next(Preprocessor *preprocessor, PPToken *token, PreprocessorError *error);

void preprocessor_destroy(const Preprocessor *preprocessor);

#endif //NA_16_PREPROCESSOR_H
