#include "token_stream.h"

Error token_stream_init(TokenStream *token_stream, const Preprocessor *preprocessor) {
    token_stream->preprocessor = *preprocessor;
    token_stream->current = (CToken){0};
    token_stream->has_current = false;

    return ERROR_OK;
}

const CToken *token_stream_peek(TokenStream *stream) {
    if (!stream->has_current) {
        token_stream_consume(stream);
    }

    return &stream->current;
}

Error token_stream_consume(TokenStream *stream) {
    PreprocessorError error;
    Error code;
    PPToken pp_token;

    if ((code = preprocessor_next(&stream->preprocessor, &pp_token, &error)) != ERROR_OK) {
        stream->has_current = false;
        return code;
    }

    if ((code = convert_ppt_to_ct(&pp_token, &stream->current)) != ERROR_OK) {
        stream->has_current = false;
        return code;
    }

    stream->has_current = true;
    return ERROR_OK;
}

Error token_stream_match(TokenStream *stream, const CTokenKind expected) {
    const CToken *token = token_stream_peek(stream);

    if (!token) {
        return ERROR_NOT_FOUND;
    }

    if (token->kind == expected) {
        token_stream_consume(stream);
        return ERROR_OK;
    }

    return ERROR_INTERNAL;
}