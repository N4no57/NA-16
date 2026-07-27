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

static char *pull_string(const PPToken *ppt) {
    const char *str_loc = &ppt->span.begin.file->contents[ppt->span.begin.offset];

    const size_t size = ppt->span.end.offset - ppt->span.begin.offset;
    char *str = malloc(size+1);

    memcpy(str, str_loc, size);
    str[size] = '\0';

    return str;
}

static void handle_identifier(const PPToken *ppt, CToken *ct) {
    char *identifier = pull_string(ppt);

    for (size_t i = 0; i < keyword_list_size; ++i) {
        if (strcmp(keywords[i], identifier) == 0) {
            ct->kind = C_TOKEN_KW_INT + (CTokenKind)i;

            free(identifier);
            return;
        }
    }

    ct->kind = C_TOKEN_IDENTIFIER;

    free(identifier);
}

static bool get_suffix(const char *text, IntegerSuffix *suffix) {
    if (!text) return false;

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
            return false;
        }
    }

    if (!result.is_unsigned &&
        (text[i] == 'u' || text[i] == 'U')) {
        result.is_unsigned = true;
        i++;
    }

    if (text[i] != '\0') {
        return false;
    }

    *suffix = result;
    return true;
}

static bool try_convert_int(const char *text, CToken *ct) { // TODO make this properly C99 compliant
    char *endptr = nullptr;
    errno = 0;
    const unsigned long long value = strtoull(text, &endptr, 0);

    if (endptr == text) {
        return false;
    }

    if (errno == ERANGE) {
        return false;
    }

    IntegerSuffix suffix;

    if (!get_suffix(endptr, &suffix)) {
        return false;
    }

    ct->kind = C_TOKEN_INTEGER_CONSTANT;
    ct->data.integer.suffix = suffix;
    ct->data.integer.unsigned_int = value;

    return true;
}

static bool convert_pp_number(const PPToken *ppt, CToken *ct) {
    // Pull the number string
    char *number = pull_string(ppt);

    // Try to match the entire spelling as C99 integer constant
    if (try_convert_int(number, ct)) {
        free(number);
        return true;
    }

    // Otherwise, try to match the entire spelling to a C99 floating constant
    // if (try_convert_float(number, ct)) { TODO actually do floating point numbers
    //     free(number);
    //     return true;
    // }

    // Consume and issue diagnostic

    free(number);
    return false;
}

bool convert_ppt_to_ct(const PPToken *ppt, CToken *ct) {
    if (!ppt || !ct) return false;
    ct->span = ppt->span;

    switch (ppt->kind) {
        case PP_TOKEN_EOF:
            ct->kind = C_TOKEN_EOF;
            return true;

        case PP_TOKEN_LEFT_BRACE:
            ct->kind = C_TOKEN_LEFT_BRACE;
            return true;

        case PP_TOKEN_RIGHT_BRACE:
            ct->kind = C_TOKEN_RIGHT_BRACE;
            return true;

        case PP_TOKEN_LEFT_PAREN:
            ct->kind = C_TOKEN_LEFT_PAREN;
            return true;

        case PP_TOKEN_RIGHT_PAREN:
            ct->kind = C_TOKEN_RIGHT_PAREN;
            return true;

        case PP_TOKEN_SEMICOLON:
            ct->kind = C_TOKEN_SEMICOLON;
            return true;

        case PP_TOKEN_COMMA:
            ct->kind = C_TOKEN_COMMA;
            return true;

        case PP_TOKEN_IDENTIFIER:
            handle_identifier(ppt, ct);
            return true;

        case PP_TOKEN_NUMBER:
            convert_pp_number(ppt, ct);
            return true;

        default:
            // there was an error but not gonna handle that as of yet
            return false;
    }
}
