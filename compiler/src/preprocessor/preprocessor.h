#ifndef NA_16_PREPROCESSOR_H
#define NA_16_PREPROCESSOR_H

#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include "macro.h"

typedef enum PPTokenSourceKind {
    PP_SOURCE_FILE,
    PP_SOURCE_MACRO_EXPANSION
} PPTokenSourceKind;

typedef struct PPTokenSource {
    PPTokenSourceKind kind;

    union {
        struct {
            Lexer lexer;
        } file;

        struct {
            const Macro *macro;
            PPToken *tokens;
            size_t count;
            size_t index;
        } expansion;
    };
} PPTokenSource;

typedef struct Preprocessor {
    PPTokenSource *sources;
    size_t stack_size;
    size_t stack_top;

    MacroTable macro_table;
} Preprocessor;

void preprocessor_init(Preprocessor *preprocessor, const Lexer *lexer);

bool preprocessor_next(Preprocessor *preprocessor, PPToken *token, LexerError *error);

void preprocessor_destroy(const Preprocessor *preprocessor);

#endif //NA_16_PREPROCESSOR_H
