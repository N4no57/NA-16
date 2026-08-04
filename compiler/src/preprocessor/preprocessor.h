#ifndef NA_16_PREPROCESSOR_H
#define NA_16_PREPROCESSOR_H

#include <error.h>
#include <vector.h>

#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include "macro.h"

typedef struct PPExpansionFrame {
    const Macro *macro;

    PPToken *tokens;
    size_t count;
    size_t index;

    bool owns_tokens;
} PPExpansionFrame;

typedef struct PPExpansionStack {
    PPExpansionFrame *sources;
    size_t stack_size;
    size_t stack_top;
} PPExpansionStack;

typedef LexerError PreprocessorError;

typedef struct Preprocessor {
    Vector files;
    Vector expansions;

    Vector macro_table;
} Preprocessor;

Error preprocessor_init(Preprocessor *preprocessor, const Lexer *lexer);

Error preprocessor_next(Preprocessor *preprocessor, PPToken *token, PreprocessorError *error);

void preprocessor_destroy(const Preprocessor *preprocessor);

#endif //NA_16_PREPROCESSOR_H
