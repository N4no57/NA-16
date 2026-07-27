#include "preprocessor.h"

void preprocessor_init(Preprocessor *preprocessor, Lexer *lexer) {
    preprocessor->lexer = lexer;
}

bool preprocessor_next(Preprocessor *preprocessor, PPToken *token, LexerError *error) {
    if (!preprocessor || !preprocessor->lexer || !token) {
        return false;
    }

    for (;;) {
        if (!lexer_next(preprocessor->lexer, token, error)) {
            return false;
        }

        if (token->kind == PP_TOKEN_NEWLINE) {
            continue;
        }

        return true;
    }
}
