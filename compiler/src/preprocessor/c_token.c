#include "c_token.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

char *(keywords[]) = {
    "int",
    "void",
    "return"
};

constexpr size_t keyword_list_size = sizeof(keywords)/sizeof(keywords[0]);

static Error handle_identifier(const PPToken *ppt, CToken *ct) {
    const char *identifier = ppt->data.string;

    for (size_t i = 0; i < keyword_list_size; ++i) {
        if (strcmp(keywords[i], identifier) == 0) {
            ct->kind = C_TOKEN_KW_INT + (CTokenKind)i;
            return ERROR_OK;
        }
    }

    ct->kind = C_TOKEN_IDENTIFIER;
    return ERROR_OK;
}

static Error get_suffix(const char *text, IntegerSuffix *suffix) {
    if (!text) return ERROR_NULL_POINTER;

    IntegerSuffix result = {
        .is_unsigned = false,
        .length = INTEGER_SUFFIX_LENGTH_NONE
    };

    size_t i = 0;

    // optional unsigned prefix
    if (text[i] == 'u' || text[i] == 'U') {
        result.is_unsigned = true;
        i++;
    }

    if (text[i] == 'l' || text[i] == 'L') {
        const char first_l = text[i++];

        result.length = INTEGER_SUFFIX_LENGTH_LONG;

        if (text[i] == first_l) {
            result.length = INTEGER_SUFFIX_LENGTH_LONG_LONG;
            i++;
        } else if (text[i] == 'l' || text[i] == 'L') {
            return ERROR_INVALID_ARGUMENT;
        }
    }

    if (!result.is_unsigned &&
        (text[i] == 'u' || text[i] == 'U')) {
        result.is_unsigned = true;
        i++;
    }

    if (text[i] == '\0') {
        return ERROR_INTERNAL;
    }

    *suffix = result;
    return ERROR_OK;
}

static Error try_convert_int(const char *text, CToken *ct) { // TODO make this properly C99 compliant
    char *endptr = nullptr;
    errno = 0;
    const unsigned long long value = strtoull(text, &endptr, 0);

    if (endptr == text) {
        return ERROR_OK; // not an int so try again
    }

    if (errno == ERANGE) {
        return ERROR_OUT_OF_RANGE;
    }

    IntegerSuffix suffix;

    const Error code = get_suffix(text, &suffix);
    if (code != ERROR_OK) {
        return code;
    }

    ct->kind = C_TOKEN_INTEGER_CONSTANT;
    ct->data.integer.suffix = suffix;
    ct->data.integer.unsigned_int = value;

    return ERROR_OK;
}

static Error convert_pp_number(const PPToken *ppt, CToken *ct) {
    // Pull the number string
    const char *number = ppt->data.string;

    // Try to match the entire spelling as C99 integer constant
    if (try_convert_int(number, ct) == ERROR_OK) {
        return ERROR_OK;
    }

    // Otherwise, try to match the entire spelling to a C99 floating constant
    // if (try_convert_float(number, ct)) { TODO actually do floating point numbers
    //     free(number);
    //     return true;
    // }

    // Consume and issue diagnostic

    return ERROR_NOT_IMPLEMENTED;
}

Error convert_ppt_to_ct(const PPToken *ppt, CToken *ct) {
    if (!ppt || !ct) return false;
    ct->span = ppt->presumed_span;

    switch (ppt->kind) {
        case PP_TOKEN_EOF:
            ct->kind = C_TOKEN_EOF;
            return ERROR_OK;

        case PP_TOKEN_LEFT_BRACE:
            ct->kind = C_TOKEN_LEFT_BRACE;
            return ERROR_OK;

        case PP_TOKEN_RIGHT_BRACE:
            ct->kind = C_TOKEN_RIGHT_BRACE;
            return ERROR_OK;

        case PP_TOKEN_LEFT_PAREN:
            ct->kind = C_TOKEN_LEFT_PAREN;
            return ERROR_OK;

        case PP_TOKEN_RIGHT_PAREN:
            ct->kind = C_TOKEN_RIGHT_PAREN;
            return ERROR_OK;

        case PP_TOKEN_SEMICOLON:
            ct->kind = C_TOKEN_SEMICOLON;
            return ERROR_OK;

        case PP_TOKEN_COMMA:
            ct->kind = C_TOKEN_COMMA;
            return ERROR_OK;

        case PP_TOKEN_IDENTIFIER:
            return handle_identifier(ppt, ct);

        case PP_TOKEN_NUMBER:
            return convert_pp_number(ppt, ct);

        default:
            // there was an error but not gonna handle that as of yet
            return ERROR_INTERNAL;
    }
}
