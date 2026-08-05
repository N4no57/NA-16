#ifndef NA_16_TOKEN_STREAM_H
#define NA_16_TOKEN_STREAM_H

#include "../preprocessor/preprocessor.h"
#include "../preprocessor/c_token.h"

typedef struct TokenStream {
    Preprocessor preprocessor;

    CToken current;
    bool has_current;
} TokenStream;

Error token_stream_init(TokenStream *token_stream, const Preprocessor *preprocessor);

const CToken *token_stream_peek(TokenStream *stream);
Error token_stream_consume(TokenStream *stream);

Error token_stream_match(TokenStream *stream, CTokenKind expected);

#endif //NA_16_TOKEN_STREAM_H
