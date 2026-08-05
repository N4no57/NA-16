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

typedef enum PPSourceKind {
    PP_FILE_SOURCE,
    PP_MACRO_SOURCE
} PPSourceKind;

typedef struct PPSourceFrame {
    PPSourceKind kind;

    union {
        Lexer file;
        PPExpansionFrame macro;
    };
} PPSourceFrame;

typedef LexerError PreprocessorError;

typedef struct Preprocessor {
    Vector sources;

    Vector macro_table;
} Preprocessor;

Error preprocessor_init(Preprocessor *preprocessor, const Lexer *lexer);

Error preprocessor_next(Preprocessor *preprocessor, PPToken *result, PreprocessorError *error);

void preprocessor_destroy(Preprocessor *preprocessor);

#endif //NA_16_PREPROCESSOR_H
