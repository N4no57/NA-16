#include <stdlib.h>
#include <error.h>
#include <string.h>

#include "preprocessor.h"
#include "c_token.h"

char *(builtin_directives[]) = {
    "__DATE__",
    "__FILE__",
    "__STDC__",
    "__STDC_HOSTED__",
    "__STDC_MB_MIGHT_NEQ_WC__",
    "__STDC_VERSION__",
    "__TIME__",
    "__STDC_IEC_599__",
    "__STDC_IEC_599_COMPLEX__"
};

Error preprocessor_init(Preprocessor *preprocessor, const Lexer *lexer) {
    if (!lexer || !preprocessor) return ERROR_NULL_POINTER;

    Error code = vector_init(&preprocessor->sources, sizeof(PPSourceFrame));
    if (code != ERROR_OK) {
        return code;
    }

    code = vector_init(&preprocessor->macro_table, sizeof(Macro));

    if (code != ERROR_OK) {
        vector_destroy(&preprocessor->sources);
        return code;
    }

    code = vector_init(&preprocessor->conditionals, sizeof(PPConditionalFrame));

    if (code != ERROR_OK) {
        vector_destroy(&preprocessor->macro_table);
        vector_destroy(&preprocessor->sources);
        return code;
    }

    code = vector_init(&preprocessor->token_queue, sizeof(PPQueueItem));

    if (code != ERROR_OK) {
        vector_destroy(&preprocessor->macro_table);
        vector_destroy(&preprocessor->sources);
        vector_destroy(&preprocessor->conditionals);
        return code;
    }

    const PPSourceFrame source = {
        .kind = PP_FILE_SOURCE,
        .file = *lexer
    };

    // init source
    vector_push(&preprocessor->sources, &source);

    return ERROR_OK;
}

static void generate_error(PreprocessorError *error, SourceSpan error_location,  char *message) {
    if (error == nullptr) return;

    *error = (PreprocessorError){
        .message = strdup(message),
        .span = error_location
    };
}

static Error pp_read_unexpanded(Preprocessor *preprocessor, PPToken *result, PPSourceKind *origin, PreprocessorError *error) {
    while (preprocessor->sources.length > 0) {
        Error code;

        if (preprocessor->token_queue.length > 0) {
            PPQueueItem item;

            code = vector_get(&preprocessor->token_queue, 0, &item);

            if (code != ERROR_OK) return code;

            *origin = item.kind;
            *result = item.token;

            vector_remove(&preprocessor->token_queue, 0, nullptr);

            return ERROR_OK;
        }

        PPSourceFrame source;
        if ((code = vector_get(&preprocessor->sources, preprocessor->sources.length-1, &source)) != ERROR_OK) {
            return code;
        }
        *origin = source.kind;

        switch (source.kind) {
            case PP_FILE_SOURCE: {
                PPToken token;

                if ((code = lexer_next(&source.file, &token, error)) != ERROR_OK) {
                    return code;
                }

                vector_set(&preprocessor->sources, preprocessor->sources.length-1, &source);

                if (token.kind != PP_TOKEN_EOF) {
                    *result = token;
                    return ERROR_OK;
                }

                // EOF from a header file
                if (preprocessor->sources.length > 1) {
                    if ((code = vector_pop(&preprocessor->sources, nullptr))) return code;
                    continue;
                }

                // EOF from the source file which means: ITS TIME TO STOP
                *result = token;
                break;
            }
            case PP_MACRO_SOURCE: {
                if (source.macro.index >= source.macro.count) {
                    if (source.macro.owns_tokens) free(source.macro.tokens);

                    if ((code = vector_pop(&preprocessor->sources, nullptr))) return code;
                    continue;
                }

                *result = source.macro.tokens[source.macro.index++];
                vector_set(&preprocessor->sources, preprocessor->sources.length-1, &source);
                break;
            }
        }

        return ERROR_OK;
    }

    return ERROR_INTERNAL; // TODO error
}

static Error pp_peek_unexpanded(Preprocessor *preprocessor, size_t lookahead, PPToken *result, PPSourceKind *origin, PreprocessorError *error) {
    Error code;

    if (preprocessor->token_queue.length > lookahead) {
        PPQueueItem item;

        code = vector_get(&preprocessor->token_queue, lookahead, &item);

        if (code != ERROR_OK) return code;

        *origin = item.kind;
        *result = item.token;

        return ERROR_OK;
    }

    PPQueueItem item = {0};

    const size_t loop_count = lookahead - preprocessor->token_queue.length;

    for (size_t i = 0; i <= loop_count; i++) {
        code = pp_read_unexpanded(preprocessor, &item.token, &item.kind, error);

        if (code != ERROR_OK) return code;

        code = vector_push(&preprocessor->token_queue, &item);

        if (code != ERROR_OK) return code;

        lookahead--;
    }

    *result = item.token;
    *origin = item.kind;

    return ERROR_OK;
}

static bool pp_active(Preprocessor *preprocessor) {
    if (preprocessor->conditionals.length > 0) {
        return ((PPConditionalFrame *)preprocessor->conditionals.data)[preprocessor->conditionals.length-1].branch_active;
    }
    return true;
}

typedef struct PPInteger {
    uintmax_t value;
    bool is_unsigned;
} PPInteger;

static bool pp_integer_truthy(PPInteger num) {
    if (num.value == 0) return false;

    return true;
}

static Error pp_process_primary_expression(const Vector *tokens, size_t *idx, const bool evaluate, PPInteger *result, PreprocessorError *error) {
    PPToken token;

    const Error code = vector_get(tokens, *idx, &token);
    if (code != ERROR_OK) return code;

    if (token.kind != PP_TOKEN_NUMBER && token.kind != PP_TOKEN_CHARACTER_CONSTANT) {
        if (error) {
            error->message = "Expected a number or character constant";
            error->span = token.presumed_span;
        }
        return ERROR_INTERNAL;
    }

    (*idx)++;
    if (*idx > tokens->length) return ERROR_OUT_OF_RANGE;

    if (!evaluate) {
        *result = (PPInteger){
            .value = (uintmax_t)0,
            .is_unsigned = false
        };

        return ERROR_OK;
    }

    if (token.kind == PP_TOKEN_CHARACTER_CONSTANT) {
        *result = (PPInteger){
            .value = (uintmax_t)*token.data.string,
            .is_unsigned = false
        };
    } else {
        CToken ct;
        convert_ppt_to_ct(&token, &ct);

        result->value = ct.data.integer.unsigned_int;
        result->is_unsigned = ct.data.integer.suffix.is_unsigned;
    }

    return ERROR_OK;
}

static Error pp_process_unary_expression(const Vector *tokens, size_t *idx, const bool evaluate, PPInteger *result, PreprocessorError *error) {
    PPToken token;

    Error code = vector_get(tokens, *idx, &token);
    if (code != ERROR_OK) return code;

    // to primary expression evaluation

    if (token.kind == PP_TOKEN_PLUS) {
        (*idx)++;
        if (*idx > tokens->length) return ERROR_OUT_OF_RANGE;

        code = vector_get(tokens, *idx, &token);
        if (code != ERROR_OK) return code;

        code = pp_process_primary_expression(tokens, idx, evaluate, result, error);
        if (code != ERROR_OK) return code;

        return ERROR_OK;
    }

    if (token.kind == PP_TOKEN_MINUS) {
        (*idx)++;
        if (*idx > tokens->length) return ERROR_OUT_OF_RANGE;

        code = vector_get(tokens, *idx, &token);
        if (code != ERROR_OK) return code;

        code = pp_process_primary_expression(tokens, idx, evaluate, result, error);
        if (code != ERROR_OK) return code;

        result->is_unsigned = true;

        return ERROR_OK;
    }

    if (token.kind == PP_TOKEN_TILDE) {
        (*idx)++;
        if (*idx > tokens->length) return ERROR_OUT_OF_RANGE;

        code = vector_get(tokens, *idx, &token);
        if (code != ERROR_OK) return code;

        code = pp_process_primary_expression(tokens, idx, evaluate, result, error);
        if (code != ERROR_OK) return code;

        result->value = ~result->value;

        return ERROR_OK;
    }

    if (token.kind == PP_TOKEN_EXCLAMATION) {
        (*idx)++;
        if (*idx > tokens->length) return ERROR_OUT_OF_RANGE;

        code = vector_get(tokens, *idx, &token);
        if (code != ERROR_OK) return code;

        code = pp_process_primary_expression(tokens, idx, evaluate, result, error);
        if (code != ERROR_OK) return code;

        result->value = !result->value;

        return ERROR_OK;
    }

    return pp_process_primary_expression(tokens, idx, evaluate, result, error);
}

static Error pp_process_multiplicative_expression(const Vector *tokens, size_t *idx, const bool evaluate, PPInteger *result, PreprocessorError *error) {
    PPInteger left;
    Error code = pp_process_unary_expression(tokens, idx, evaluate, &left, error);
    if (code != ERROR_OK) return code;

    PPToken token;

    code = vector_get(tokens, *idx, &token);
    if (code == ERROR_OUT_OF_RANGE) {
        *result = left;
        return ERROR_OK;
    }
    if (code != ERROR_OK) return code;

    while (
        token.kind == PP_TOKEN_ASTERISK ||
        token.kind == PP_TOKEN_SLASH ||
        token.kind == PP_TOKEN_PERCENT
    ) {
        const PPTokenKind operator = token.kind;

        (*idx)++;
        if (*idx > tokens->length) {
            break;
        }

        PPInteger right;
        code = pp_process_multiplicative_expression(tokens, idx, evaluate, &right, error);
        if (code != ERROR_OK) return code;

        switch (operator) {
            case PP_TOKEN_ASTERISK:
                left.value = left.value * right.value;
                break;
            case PP_TOKEN_SLASH:
                left.value = left.value / right.value;
                break;
            case PP_TOKEN_PERCENT:
                left.value = left.value % right.value;
                break;
            default:
                return ERROR_INTERNAL; // TODO error
        }

        code = vector_get(tokens, *idx, &token);
        if (code == ERROR_OUT_OF_RANGE) break;
        if (code != ERROR_OK) return code;
    }

    *result = left;
    return ERROR_OK;
}

static Error pp_process_additive_expression(const Vector *tokens, size_t *idx, const bool evaluate, PPInteger *result, PreprocessorError *error) {
    PPInteger left;
    Error code = pp_process_multiplicative_expression(tokens, idx, evaluate, &left, error);
    if (code != ERROR_OK) return code;

    PPToken token;

    code = vector_get(tokens, *idx, &token);
    if (code == ERROR_OUT_OF_RANGE) {
        *result = left;
        return ERROR_OK;
    }
    if (code != ERROR_OK) return code;

    while (
        token.kind == PP_TOKEN_PLUS ||
        token.kind == PP_TOKEN_MINUS
    ) {
        const PPTokenKind operator = token.kind;

        (*idx)++;
        if (*idx > tokens->length) {
            break;
        }

        PPInteger right;
        code = pp_process_additive_expression(tokens, idx, evaluate, &right, error);
        if (code != ERROR_OK) return code;

        switch (operator) {
            case PP_TOKEN_PLUS:
                left.value = left.value + right.value;
                break;
            case PP_TOKEN_MINUS:
                left.value = left.value - right.value;
                break;
            default:
                return ERROR_INTERNAL; // TODO error
        }

        code = vector_get(tokens, *idx, &token);
        if (code == ERROR_OUT_OF_RANGE) break;
        if (code != ERROR_OK) return code;
    }

    *result = left;
    return ERROR_OK;
}

static Error pp_process_shift_expression(const Vector *tokens, size_t *idx, const bool evaluate, PPInteger *result, PreprocessorError *error) {
    PPInteger left;
    Error code = pp_process_additive_expression(tokens, idx, evaluate, &left, error);
    if (code != ERROR_OK) return code;

    PPToken token;

    code = vector_get(tokens, *idx, &token);
    if (code == ERROR_OUT_OF_RANGE) {
        *result = left;
        return ERROR_OK;
    }
    if (code != ERROR_OK) return code;

    while (
        token.kind == PP_TOKEN_SHIFT_LEFT ||
        token.kind == PP_TOKEN_SHIFT_RIGHT
    ) {
        const PPTokenKind operator = token.kind;

        (*idx)++;
        if (*idx > tokens->length) {
            break;
        }

        PPInteger right;
        code = pp_process_shift_expression(tokens, idx, evaluate, &right, error);
        if (code != ERROR_OK) return code;

        switch (operator) {
            case PP_TOKEN_SHIFT_LEFT:
                left.value = left.value <<  right.value;
                break;
            case PP_TOKEN_SHIFT_RIGHT:
                left.value = left.value >> right.value;
                break;
            default:
                return ERROR_INTERNAL; // TODO error
        }

        code = vector_get(tokens, *idx, &token);
        if (code == ERROR_OUT_OF_RANGE) break;
        if (code != ERROR_OK) return code;
    }

    *result = left;
    return ERROR_OK;
}

static Error pp_process_relational_expression(const Vector *tokens, size_t *idx, const bool evaluate, PPInteger *result, PreprocessorError *error) {
    PPInteger left;
    Error code = pp_process_shift_expression(tokens, idx, evaluate, &left, error);
    if (code != ERROR_OK) return code;

    PPToken token;

    code = vector_get(tokens, *idx, &token);
    if (code == ERROR_OUT_OF_RANGE) {
        *result = left;
        return ERROR_OK;
    }
    if (code != ERROR_OK) return code;

    while (
        token.kind == PP_TOKEN_LESS ||
        token.kind == PP_TOKEN_LESS_EQUAL ||
        token.kind == PP_TOKEN_GREATER ||
        token.kind == PP_TOKEN_GREATER_EQUAL
    ) {
        const PPTokenKind operator = token.kind;

        (*idx)++;
        if (*idx > tokens->length) {
            break;
        }

        PPInteger right;
        code = pp_process_relational_expression(tokens, idx, evaluate, &right, error);
        if (code != ERROR_OK) return code;

        switch (operator) {
            case PP_TOKEN_LESS:
                left.value = left.value < right.value;
                break;
            case PP_TOKEN_LESS_EQUAL:
                left.value = left.value <= right.value;
                break;
            case PP_TOKEN_GREATER:
                left.value = left.value > right.value;
                break;
            case PP_TOKEN_GREATER_EQUAL:
                left.value = left.value >= right.value;
                break;
            default:
                return ERROR_INTERNAL; // TODO error
        }

        code = vector_get(tokens, *idx, &token);
        if (code == ERROR_OUT_OF_RANGE) break;
        if (code != ERROR_OK) return code;
    }

    *result = left;
    return ERROR_OK;
}

static Error pp_process_equality_expression(const Vector *tokens, size_t *idx, const bool evaluate, PPInteger *result, PreprocessorError *error) {
    PPInteger left;
    Error code = pp_process_relational_expression(tokens, idx, evaluate, &left, error);
    if (code != ERROR_OK) return code;

    PPToken token;

    code = vector_get(tokens, *idx, &token);
    if (code == ERROR_OUT_OF_RANGE) {
        *result = left;
        return ERROR_OK;
    }
    if (code != ERROR_OK) return code;

    while (
        token.kind == PP_TOKEN_EQUAL_EQUAL ||
        token.kind == PP_TOKEN_NOT_EQUAL
    ) {
        const PPTokenKind operator = token.kind;

        (*idx)++;
        if (*idx > tokens->length) {
            break;
        }

        PPInteger right;
        code = pp_process_equality_expression(tokens, idx, evaluate, &right, error);
        if (code != ERROR_OK) return code;

        switch (operator) {
            case PP_TOKEN_EQUAL_EQUAL:
                left.value = left.value == right.value;
                break;
            case PP_TOKEN_NOT_EQUAL:
                left.value = left.value != right.value;
                break;
            default:
                return ERROR_INTERNAL; // TODO error
        }

        code = vector_get(tokens, *idx, &token);
        if (code == ERROR_OUT_OF_RANGE) break;
        if (code != ERROR_OK) return code;
    }

    *result = left;
    return ERROR_OK;
}

static Error pp_process_and_expression(const Vector *tokens, size_t *idx, const bool evaluate, PPInteger *result, PreprocessorError *error) {
    PPInteger left;
    Error code = pp_process_equality_expression(tokens, idx, evaluate, &left, error);
    if (code != ERROR_OK) return code;

    PPToken token;

    code = vector_get(tokens, *idx, &token);
    if (code == ERROR_OUT_OF_RANGE) {
        *result = left;
        return ERROR_OK;
    }
    if (code != ERROR_OK) return code;

    while (
        token.kind == PP_TOKEN_AMPERSAND
    ) {
        const PPTokenKind operator = token.kind;

        (*idx)++;
        if (*idx > tokens->length) {
            break;
        }

        PPInteger right;
        code = pp_process_and_expression(tokens, idx, evaluate, &right, error);
        if (code != ERROR_OK) return code;

        switch (operator) {
            case PP_TOKEN_AMPERSAND:
                left.value = left.value & right.value;
                break;
            default:
                return ERROR_INTERNAL; // TODO error
        }

        code = vector_get(tokens, *idx, &token);
        if (code == ERROR_OUT_OF_RANGE) break;
        if (code != ERROR_OK) return code;
    }

    *result = left;
    return ERROR_OK;
}

static Error pp_process_xor_expression(const Vector *tokens, size_t *idx, const bool evaluate, PPInteger *result, PreprocessorError *error) {
    PPInteger left;
    Error code = pp_process_and_expression(tokens, idx, evaluate, &left, error);
    if (code != ERROR_OK) return code;

    PPToken token;

    code = vector_get(tokens, *idx, &token);
    if (code == ERROR_OUT_OF_RANGE) {
        *result = left;
        return ERROR_OK;
    }
    if (code != ERROR_OK) return code;

    while (
        token.kind == PP_TOKEN_CARET
    ) {
        const PPTokenKind operator = token.kind;

        (*idx)++;
        if (*idx > tokens->length) {
            break;
        }

        PPInteger right;
        code = pp_process_xor_expression(tokens, idx, evaluate, &right, error);
        if (code != ERROR_OK) return code;

        switch (operator) {
            case PP_TOKEN_CARET:
                left.value = left.value ^ right.value;
                break;
            default:
                return ERROR_INTERNAL; // TODO error
        }

        code = vector_get(tokens, *idx, &token);
        if (code == ERROR_OUT_OF_RANGE) break;
        if (code != ERROR_OK) return code;
    }

    *result = left;
    return ERROR_OK;
}

static Error pp_process_or_expression(const Vector *tokens, size_t *idx, const bool evaluate, PPInteger *result, PreprocessorError *error) {
    PPInteger left;
    Error code = pp_process_xor_expression(tokens, idx, evaluate, &left, error);
    if (code != ERROR_OK) return code;

    PPToken token;

    code = vector_get(tokens, *idx, &token);
    if (code == ERROR_OUT_OF_RANGE) {
        *result = left;
        return ERROR_OK;
    }
    if (code != ERROR_OK) return code;

    while (
        token.kind == PP_TOKEN_PIPE
    ) {
        const PPTokenKind operator = token.kind;

        (*idx)++;
        if (*idx > tokens->length) {
            break;
        }

        PPInteger right;
        code = pp_process_or_expression(tokens, idx, evaluate, &right, error);
        if (code != ERROR_OK) return code;

        switch (operator) {
            case PP_TOKEN_PIPE:
                left.value = left.value | right.value;
                break;
            default:
                return ERROR_INTERNAL; // TODO error
        }

        code = vector_get(tokens, *idx, &token);
        if (code == ERROR_OUT_OF_RANGE) break;
        if (code != ERROR_OK) return code;
    }

    *result = left;
    return ERROR_OK;
}

static Error pp_process_logical_and_expression(const Vector *tokens, size_t *idx, const bool evaluate, PPInteger *result, PreprocessorError *error) {
    PPInteger left;
    Error code = pp_process_or_expression(tokens, idx, evaluate, &left, error);
    if (code != ERROR_OK) return code;

    PPToken token;

    code = vector_get(tokens, *idx, &token);
    if (code == ERROR_OUT_OF_RANGE) {
        *result = left;
        return ERROR_OK;
    }
    if (code != ERROR_OK) return code;

    while (
        token.kind == PP_TOKEN_LOGICAL_AND
    ) {
        const PPTokenKind operator = token.kind;

        (*idx)++;
        if (*idx > tokens->length) {
            break;
        }

        PPInteger right;
        const bool to_eval = evaluate && pp_integer_truthy(left);
        code = pp_process_logical_and_expression(tokens, idx, evaluate, &right, error);
        if (code != ERROR_OK) return code;

        switch (operator) {
            case PP_TOKEN_LOGICAL_AND:
                left.value = left.value && right.value;
                break;
            default:
                return ERROR_INTERNAL; // TODO error
        }

        code = vector_get(tokens, *idx, &token);
        if (code == ERROR_OUT_OF_RANGE) break;
        if (code != ERROR_OK) return code;
    }

    *result = left;
    return ERROR_OK;
}

static Error pp_process_logical_or_expression(const Vector *tokens, size_t *idx, const bool evaluate, PPInteger *result, PreprocessorError *error) {
    PPInteger left;
    Error code = pp_process_logical_and_expression(tokens, idx, evaluate, &left, error);
    if (code != ERROR_OK) return code;

    PPToken token;

    code = vector_get(tokens, *idx, &token);
    if (code == ERROR_OUT_OF_RANGE) {
        *result = left;
        return ERROR_OK;
    }
    if (code != ERROR_OK) return code;

    while (
        token.kind == PP_TOKEN_LOGICAL_OR
    ) {
        const PPTokenKind operator = token.kind;

        (*idx)++;
        if (*idx > tokens->length) {
            break;
        }

        PPInteger right;
        const bool to_eval = evaluate && !pp_integer_truthy(left);
        code = pp_process_logical_or_expression(tokens, idx, to_eval, &right, error);
        if (code != ERROR_OK) return code;

        switch (operator) {
            case PP_TOKEN_LOGICAL_OR:
                left.value = left.value || right.value;
                break;
            default:
                return ERROR_INTERNAL; // TODO error
        }

        code = vector_get(tokens, *idx, &token);
        if (code == ERROR_OUT_OF_RANGE) break;
        if (code != ERROR_OK) return code;
    }

    *result = left;
    return ERROR_OK;
}

static Error pp_process_conditional_expression(const Vector *tokens, size_t *idx, const bool evaluate, PPInteger *result, PreprocessorError *error) {
    PPInteger condition;
    Error code = pp_process_logical_or_expression(tokens, idx, evaluate, &condition, error);
    if (code != ERROR_OK) return code;

    PPToken token;

    code = vector_get(tokens, *idx, &token);
    if (code == ERROR_OUT_OF_RANGE) {
        *result = condition;
        return ERROR_OK;
    }
    if (code != ERROR_OK) return code;

    if (token.kind != PP_TOKEN_QUESTION) { // TODO error
        return ERROR_INTERNAL;
    }

    (*idx)++;
    if (*idx > tokens->length) return ERROR_OUT_OF_RANGE;

    PPInteger true_val;

    code = pp_process_conditional_expression(tokens, idx, evaluate, &true_val, error);
    if (code != ERROR_OK) return code;

    (*idx)++;
    if (*idx > tokens->length) return ERROR_OUT_OF_RANGE;

    code = vector_get(tokens, *idx, &token);
    if (code != ERROR_OK) return code;

    if (token.kind != PP_TOKEN_COLON) { // TODO error
        return ERROR_INTERNAL;
    }

    PPInteger false_val;

    code = pp_process_conditional_expression(tokens, idx, evaluate, &false_val, error);
    if (code != ERROR_OK) return code;

    (*idx)++;
    if (*idx > tokens->length) return ERROR_OUT_OF_RANGE;

    *result = condition.value ? true_val : false_val;

    return ERROR_OK;
}

static Error pp_process_condition(Preprocessor *preprocessor, bool *condition, PreprocessorError *error) {
    Error code;

    // collect conditional
    Vector token_buffer;
    code = vector_init(&token_buffer, sizeof(PPToken));

    if (code != ERROR_OK) return code;

    PPToken token;
    PPSourceKind origin;

    code = pp_read_unexpanded(preprocessor, &token, &origin, error);

    if (code != ERROR_OK) return code;

    while (token.kind != PP_TOKEN_NEWLINE && token.kind != PP_TOKEN_EOF) {
        if (origin != PP_FILE_SOURCE) { // TODO error
            vector_destroy(&token_buffer);
            return ERROR_INTERNAL;
        }

        code = vector_push(&token_buffer, &token);
        if (code != ERROR_OK) return code;

        code = pp_read_unexpanded(preprocessor, &token, &origin, error);
        if (code != ERROR_OK) return code;
    }

    // replace defined() functions
    size_t i = 0;
    for (i = 0; i < token_buffer.length; i++) {
        code = vector_get(&token_buffer, i, &token);

        if (code != ERROR_OK) return code;

        if (token.kind == PP_TOKEN_IDENTIFIER && strcmp(token.data.string, "defined") == 0) {
            const size_t start = i;
            const SourceSpan actual_span = token.actual_span;

            i++;
            if (i > token_buffer.length) return ERROR_OUT_OF_RANGE;

            code = vector_get(&token_buffer, i, &token);
            if (code != ERROR_OK) return code;

            if (token.kind != PP_TOKEN_LEFT_PAREN) { // TODO error
                return ERROR_INTERNAL;
            }

            i++;
            if (i > token_buffer.length) return ERROR_OUT_OF_RANGE;

            code = vector_get(&token_buffer, i, &token);
            if (code != ERROR_OK) return code;

            if (token.kind != PP_TOKEN_IDENTIFIER) { // TODO error
                return ERROR_INTERNAL;
            }

            const void *macro = macro_table_find(&preprocessor->macro_table, token.data.string);

            const bool defined = macro != nullptr;

            i++;
            if (i > token_buffer.length) return ERROR_OUT_OF_RANGE;

            code = vector_get(&token_buffer, i, &token);

            if (code != ERROR_OK) return code;

            if (token.kind != PP_TOKEN_RIGHT_PAREN) { // TODO error
                return ERROR_INTERNAL;
            }

            i++;
            if (i > token_buffer.length) return ERROR_OUT_OF_RANGE;

            const size_t end = i;

            PPToken tok = {
                .kind = PP_TOKEN_NUMBER,
                .actual_span = actual_span,
                .presumed_span = actual_span,
                .leading_space = false,
                .start_of_line = false,
                .wide = false,
                .data = {nullptr}
            };

            tok.data.string = defined ? strdup("1") : strdup("0");

            vector_set(&token_buffer, start, &tok);

            for (size_t j = 1; j < end - start; j++) {
                code = vector_remove(&token_buffer, start+1, nullptr);
                if (code != ERROR_OK) return code;
            }

            i = start;
        }
    }

    // macro expand macros
    i = 0;
    while (i < token_buffer.length) { // expand macros
        code = vector_get(&token_buffer, i, &token);
        if (code != ERROR_OK) return code;

        if (token.kind == PP_TOKEN_IDENTIFIER) {
            Macro *macro = macro_table_find(&preprocessor->macro_table, token.data.string);

            if (macro == nullptr) {
                PPToken tok = {
                    .kind = PP_TOKEN_NUMBER,
                    .leading_space = false,
                    .start_of_line = false,
                    .wide = false,
                    .data = {strdup("0")}
                };

                vector_set(&token_buffer, i, &tok);

                i++;
                continue;
            }

            if (macro->kind == MACRO_OBJECT_LIKE) {
                code = vector_remove(&token_buffer, i, nullptr);
                if (code != ERROR_OK) return code;

                for (size_t j = 0; j < macro->replacement_count; j++) {
                    code = vector_insert(&token_buffer, i+j, &macro->replacement[j]);

                    if (code != ERROR_OK) return code;
                }
            } else if (macro->kind == MACRO_FUNCTION_LIKE) {

            }

            continue;
        }

        i++;
    }

    PPInteger num;

    size_t idx = 0;
    code = pp_process_conditional_expression(&token_buffer, &idx, true, &num, error);

    if (code != ERROR_OK) return code;

    *condition = pp_integer_truthy(num);

    return ERROR_OK;
}

static Error pp_process_def(Preprocessor *preprocessor, bool *condition, bool not_def, PreprocessorError *error) {
    PPToken token;
    PPSourceKind origin;

    Error code = pp_read_unexpanded(preprocessor, &token, &origin, error);
    if (code != ERROR_OK) return code;
    if (origin != PP_FILE_SOURCE) { // TODO error
        return ERROR_INTERNAL;
    }

    if (token.kind != PP_TOKEN_IDENTIFIER) { // TODO error
        return ERROR_INTERNAL;
    }

    Macro *macro = macro_table_find(&preprocessor->macro_table, token.data.string);

    if (not_def) *condition = macro == nullptr;
    else *condition = macro != nullptr;

    code = pp_read_unexpanded(preprocessor, &token, &origin, error);
    if (code != ERROR_OK) return code;
    if (origin != PP_FILE_SOURCE) { // TODO error
        return ERROR_INTERNAL;
    }

    if (token.kind != PP_TOKEN_NEWLINE) { // TODO error
        return ERROR_INTERNAL;
    }

    return ERROR_OK;
}

static Error pp_process_directive(Preprocessor *preprocessor, PreprocessorError *error) {
    Error code;
    PPToken token;
    PPSourceKind origin;

    if ((code = pp_read_unexpanded(preprocessor, &token, &origin, error)) != ERROR_OK) return code;

    if (origin != PP_FILE_SOURCE) { // TODO error
        return ERROR_INTERNAL;
    }

    if (token.kind == PP_TOKEN_NEWLINE) {
        return ERROR_OK;
    }

    if (token.kind != PP_TOKEN_IDENTIFIER) { // TODO error
        return ERROR_INTERNAL;
    }

    if (strcmp(token.data.string, "if") == 0) {
        const bool parent_active = pp_active(preprocessor);
        bool condition = false;

        if (parent_active) {
            if ((code = pp_process_condition(preprocessor, &condition, error)) != ERROR_OK) {
                return code;
            }
        }

        const PPConditionalFrame conditional = {
            .parent_active = parent_active,
            .branch_active = parent_active && condition,
            .branch_taken = parent_active && condition,
            .saw_else = false
        };

        return vector_push(&preprocessor->conditionals, &conditional);
    }

    if (strcmp(token.data.string, "ifdef") == 0) {
        const bool parent_active = pp_active(preprocessor);
        bool condition = false;

        if (parent_active) {
            pp_process_def(preprocessor, &condition, false, error);
        }

        const PPConditionalFrame conditional = {
            .parent_active = parent_active,
            .branch_active = parent_active && condition,
            .branch_taken = parent_active && condition,
            .saw_else = false
        };

        return vector_push(&preprocessor->conditionals, &conditional);
    }

    if (strcmp(token.data.string, "ifndef") == 0) {
        const bool parent_active = pp_active(preprocessor);
        bool condition = false;

        if (parent_active) {
            pp_process_def(preprocessor, &condition, true, error);
        }

        const PPConditionalFrame conditional = {
            .parent_active = parent_active,
            .branch_active = parent_active && condition,
            .branch_taken = parent_active && condition,
            .saw_else = false
        };

        return vector_push(&preprocessor->conditionals, &conditional);
    }

    if (strcmp(token.data.string, "elif") == 0) {
        PPConditionalFrame frame;

        if ((code = vector_get(&preprocessor->conditionals, preprocessor->conditionals.length-1, &frame)) != ERROR_OK) return code;

        if (frame.saw_else) { // TODO error
            return ERROR_INTERNAL;
        }

        bool condition = false;

        if (frame.parent_active && !frame.branch_taken) {
            condition = pp_process_condition(preprocessor, &condition, error);
        }

        frame.branch_active =
            frame.parent_active &&
            !frame.branch_taken &&
            condition;

        if (frame.branch_active) frame.branch_taken = true;

        return vector_set(&preprocessor->conditionals, preprocessor->conditionals.length-1, &frame);
    }

    if (strcmp(token.data.string, "else") == 0) {
        PPConditionalFrame frame;

        if ((code = vector_get(&preprocessor->conditionals, preprocessor->conditionals.length-1, &frame)) != ERROR_OK) return code;

        if (frame.saw_else) { // TODO error
            return ERROR_INTERNAL;
        }

        frame.saw_else = true;

        frame.branch_active =
            frame.parent_active &&
            !frame.branch_taken;

            frame.branch_taken = true;

        return vector_set(&preprocessor->conditionals, preprocessor->conditionals.length-1, &frame);
    }

    if (strcmp(token.data.string, "endif") == 0) {
        if (preprocessor->conditionals.length == 0) { // TODO error
            return ERROR_INTERNAL;
        }

        return vector_pop(&preprocessor->conditionals, nullptr);
    }

    if (!pp_active(preprocessor)) return ERROR_OK;

    if (strcmp(token.data.string, "define") == 0) {
        if ((code = pp_read_unexpanded(preprocessor, &token, &origin, error)) != ERROR_OK) return code;

        if (token.kind != PP_TOKEN_IDENTIFIER || origin != PP_FILE_SOURCE) { // TODO error
            return ERROR_INTERNAL;
        }

        Macro macro = {0};
        macro.name = strdup(token.data.string);

        if ((code = pp_read_unexpanded(preprocessor, &token, &origin, error)) != ERROR_OK) return code;

        // check if object like macro or function like macro
        if (token.kind == PP_TOKEN_LEFT_PAREN && !token.leading_space) {
            // function like
            macro.kind = MACRO_FUNCTION_LIKE;
        } else {
            // object like
            macro.kind = MACRO_OBJECT_LIKE;

            Vector tokens;
            vector_init(&tokens, sizeof(PPToken));

            while (token.kind != PP_TOKEN_NEWLINE) {
                if (origin != PP_FILE_SOURCE) { // TODO error
                    vector_destroy(&tokens);
                    free(macro.name);
                    return ERROR_INTERNAL;
                }

                vector_push(&tokens, &token);

                if ((code = pp_read_unexpanded(preprocessor, &token, &origin, error)) != ERROR_OK) return code;
            }

            macro.replacement_count = tokens.length;
            macro.replacement = (PPToken *)tokens.data;

            return macro_table_define(&preprocessor->macro_table, macro);
        }
    } else if (strcmp(token.data.string, "undef") == 0) {
        if ((code = pp_read_unexpanded(preprocessor, &token, &origin, error)) != ERROR_OK) return code;

        if (token.kind != PP_TOKEN_IDENTIFIER || origin != PP_FILE_SOURCE) { // TODO error
            return ERROR_INTERNAL;
        }

        macro_table_undef(&preprocessor->macro_table, token.data.string);
        return ERROR_OK;
    } else if (strcmp(token.data.string, "include") == 0) {

    } else if (strcmp(token.data.string, "line") == 0) {
        // TODO needs to be done for C99 compliance
    } else if (strcmp(token.data.string, "error") == 0) {
        // TODO needs to be done for C99 compliance
    } else if (strcmp(token.data.string, "pragma") == 0) {
        // TODO needs to be done for C99 compliance
    }

    return ERROR_OK;
}

static Error pp_try_expand_macro(Preprocessor *preprocessor, PPToken *identifier, bool *expanded) {
    const Macro *macro = macro_table_find(&preprocessor->macro_table, identifier->data.string);

    if (macro == nullptr) {
        *expanded = false;
        return ERROR_OK;
    }

    if (macro->kind == MACRO_OBJECT_LIKE) {
        PPSourceFrame expansion = {0};

        expansion.kind = PP_MACRO_SOURCE;
        expansion.macro.macro = macro;
        expansion.macro.count = macro->replacement_count;
        expansion.macro.tokens = macro->replacement;

        {
            PPToken *replacement = &macro->replacement[0];
            size_t size = replacement->actual_span.end.offset - replacement->actual_span.begin.offset;

            replacement->presumed_span = identifier->presumed_span;

            replacement->presumed_span.end.offset = replacement->presumed_span.begin.offset + size;

            for (size_t i = 1; i < macro->replacement_count; i++) {
                replacement = &macro->replacement[i];
                size = replacement->actual_span.end.offset - replacement->actual_span.begin.offset;

                replacement->presumed_span.begin = macro->replacement[i-1].presumed_span.end;
                if (replacement->leading_space) replacement->presumed_span.begin.offset++;

                replacement->presumed_span.end.offset = replacement->presumed_span.begin.offset + size;
            }
        }

        const Error code = vector_push(&preprocessor->sources, &expansion);

        if (code != ERROR_OK) return code;
    } else if (macro->kind == MACRO_FUNCTION_LIKE) {

    }

    *expanded = true;
    return ERROR_OK;
}

static Error pp_collect_arguments(...);
static Error pp_generate_replacement(...);
static Error pp_push_source(...);
static Error pp_is_active(...);

Error preprocessor_next(Preprocessor *preprocessor, PPToken *result, PreprocessorError *error) {
    if (!preprocessor || !result) {
        return ERROR_NULL_POINTER;
    }

    for (;;) {
        Error code;
        PPToken token;
        PPSourceKind source;

        if ((code = pp_read_unexpanded(preprocessor, &token, &source, error)) != ERROR_OK) {
            return code;
        }

        if (token.kind == PP_TOKEN_EOF) {
            if (preprocessor->conditionals.length != 0) { // TODO error
                return ERROR_INTERNAL;
            }

            *result = token;
            return ERROR_OK;
        }

        // only raw files can create preprocessing directives
        if (source == PP_FILE_SOURCE && token.kind == PP_TOKEN_HASH && token.start_of_line) {
            if ((code = pp_process_directive(preprocessor, error)) != ERROR_OK) {
                return code;
            }

            continue;
        }

        if (!pp_active(preprocessor)) {
            continue;
        }

        if (token.kind == PP_TOKEN_NEWLINE) {
            continue;
        }

        if (token.kind == PP_TOKEN_IDENTIFIER) {
            bool expanded;

            pp_try_expand_macro(preprocessor, &token, &expanded);

            if (expanded) continue;
        }

        *result = token;
        return ERROR_OK;
    }
}

void preprocessor_destroy(Preprocessor *preprocessor) {
    vector_destroy(&preprocessor->sources);

    // TODO destroy macro table

    vector_destroy(&preprocessor->macro_table);
}
