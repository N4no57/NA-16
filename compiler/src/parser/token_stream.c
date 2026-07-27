#include "token_stream.h"

void token_stream_init(TokenStream *token_stream, const Preprocessor *preprocessor) {
    token_stream->preprocessor = *preprocessor;
    token_stream->current = (CToken){0};
    token_stream->has_current = false;
}

const CToken *token_stream_peek(TokenStream *stream) {
    if (!stream->has_current) {
        token_stream_consume(stream);
    }

    return &stream->current;
}

bool token_stream_consume(TokenStream *stream) {
    LexerError error;
    PPToken pp_token;

    if (!preprocessor_next(&stream->preprocessor, &pp_token, &error)) {
        stream->has_current = false;
        return false;
    }

    if (!convert_ppt_to_ct(&pp_token, &stream->current)) {
        stream->has_current = false;
        return false;
    }

    stream->has_current = true;
    return true;
}

bool token_stream_match(TokenStream *stream, const CTokenKind expected) {
    const CToken *token = token_stream_peek(stream);

    if (!token) {
        return false;
    }

    if (token->kind == expected) {
        token_stream_consume(stream);
        return true;
    }

    return false;
}