#include "token_stream.h"

Error token_stream_init(TokenStream *token_stream, const Preprocessor *preprocessor) {
    token_stream->preprocessor = *preprocessor;

    return vector_init(&token_stream->token_buffer, sizeof(CToken));
}

static Error token_stream_generate(TokenStream *stream, CToken *result) {
    CToken token;
    PreprocessorError error;
    Error code;
    PPToken pp_token;

    if ((code = preprocessor_next(&stream->preprocessor, &pp_token, &error)) != ERROR_OK) {
        return code;
    }

    if ((code = convert_ppt_to_ct(&pp_token, &token)) != ERROR_OK) {
        return code;
    }

    *result = token;
    return ERROR_OK;
}

const CToken *token_stream_peek(TokenStream *stream, size_t lookahead) {
    for (size_t i = stream->token_buffer.length; i <= lookahead; i++) {
        CToken token;
        if (token_stream_generate(stream, &token) != ERROR_OK) return nullptr;

        if (vector_push(&stream->token_buffer, &token) != ERROR_OK) return nullptr;
    }

    return &((CToken *)stream->token_buffer.data)[lookahead];
}

Error token_stream_consume(TokenStream *stream, CToken *result) {
    if (stream->token_buffer.length > 0) {
        return vector_remove(&stream->token_buffer, 0, result);
    }

    CToken token;

    const Error code = token_stream_generate(stream, &token);
    if (code != ERROR_OK) return code;

    if (result != nullptr) *result = token;

    return ERROR_OK;
}

Error token_stream_match(TokenStream *stream, const CTokenKind expected) {
    const CToken *token = token_stream_peek(stream, 0);

    if (!token) {
        return ERROR_NOT_FOUND;
    }

    if (token->kind == expected) {
        token_stream_consume(stream, nullptr);
        return ERROR_OK;
    }

    return ERROR_INTERNAL;
}