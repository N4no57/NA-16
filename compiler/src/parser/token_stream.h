#ifndef NA_16_TOKEN_STREAM_H
#define NA_16_TOKEN_STREAM_H

#include "../preprocessor/preprocessor.h"
#include "../preprocessor/c_token.h"

typedef struct TokenStream {
    Preprocessor preprocessor;

    CToken current;
    bool has_current;
} TokenStream;

void token_stream_init(TokenStream *token_stream, Preprocessor *preprocessor);

const CToken *token_stream_peek(TokenStream *stream);
bool token_stream_consume(TokenStream *stream);

bool token_stream_match(TokenStream *stream, CToken expected);

#endif //NA_16_TOKEN_STREAM_H
