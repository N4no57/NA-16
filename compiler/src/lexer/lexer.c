#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct PunctuatorEntry {
    const char *spelling;
    size_t length;
    PPTokenKind kind;
} PunctuatorEntry;

#define PUNCTUATOR(text, token_kind) \
    { text, sizeof(text) - 1, token_kind }

static const PunctuatorEntry punctuators[] = {
    PUNCTUATOR("%:%:", PP_TOKEN_HASH_HASH),

    PUNCTUATOR("<<=", PP_TOKEN_SHIFT_LEFT_ASSIGN),
    PUNCTUATOR(">>=", PP_TOKEN_SHIFT_RIGHT_ASSIGN),
    PUNCTUATOR("...", PP_TOKEN_ELLIPSIS),

    PUNCTUATOR("->", PP_TOKEN_ARROW),
    PUNCTUATOR("++", PP_TOKEN_INCREMENT),
    PUNCTUATOR("--", PP_TOKEN_DECREMENT),
    PUNCTUATOR("<<", PP_TOKEN_SHIFT_LEFT),
    PUNCTUATOR(">>", PP_TOKEN_SHIFT_RIGHT),
    PUNCTUATOR("<=", PP_TOKEN_LESS_EQUAL),
    PUNCTUATOR(">=", PP_TOKEN_GREATER_EQUAL),
    PUNCTUATOR("==", PP_TOKEN_EQUAL_EQUAL),
    PUNCTUATOR("!=", PP_TOKEN_NOT_EQUAL),
    PUNCTUATOR("&&", PP_TOKEN_LOGICAL_AND),
    PUNCTUATOR("||", PP_TOKEN_LOGICAL_OR),
    PUNCTUATOR("*=", PP_TOKEN_MULTIPLY_ASSIGN),
    PUNCTUATOR("/=", PP_TOKEN_DIVIDE_ASSIGN),
    PUNCTUATOR("%=", PP_TOKEN_REMAINDER_ASSIGN),
    PUNCTUATOR("+=", PP_TOKEN_ADD_ASSIGN),
    PUNCTUATOR("-=", PP_TOKEN_SUBTRACT_ASSIGN),
    PUNCTUATOR("&=", PP_TOKEN_AND_ASSIGN),
    PUNCTUATOR("^=", PP_TOKEN_XOR_ASSIGN),
    PUNCTUATOR("|=", PP_TOKEN_OR_ASSIGN),

    PUNCTUATOR("<:", PP_TOKEN_LEFT_BRACKET),
    PUNCTUATOR(":>", PP_TOKEN_RIGHT_BRACKET),
    PUNCTUATOR("<%", PP_TOKEN_LEFT_BRACE),
    PUNCTUATOR("%>", PP_TOKEN_RIGHT_BRACE),
    PUNCTUATOR("%:", PP_TOKEN_HASH),
    PUNCTUATOR("##", PP_TOKEN_HASH_HASH),

    PUNCTUATOR("[", PP_TOKEN_LEFT_BRACKET),
    PUNCTUATOR("]", PP_TOKEN_RIGHT_BRACKET),
    PUNCTUATOR("(", PP_TOKEN_LEFT_PAREN),
    PUNCTUATOR(")", PP_TOKEN_RIGHT_PAREN),
    PUNCTUATOR("{", PP_TOKEN_LEFT_BRACE),
    PUNCTUATOR("}", PP_TOKEN_RIGHT_BRACE),
    PUNCTUATOR(".", PP_TOKEN_DOT),
    PUNCTUATOR("&", PP_TOKEN_AMPERSAND),
    PUNCTUATOR("*", PP_TOKEN_ASTERISK),
    PUNCTUATOR("+", PP_TOKEN_PLUS),
    PUNCTUATOR("-", PP_TOKEN_MINUS),
    PUNCTUATOR("~", PP_TOKEN_TILDE),
    PUNCTUATOR("!", PP_TOKEN_EXCLAMATION),
    PUNCTUATOR("/", PP_TOKEN_SLASH),
    PUNCTUATOR("%", PP_TOKEN_PERCENT),
    PUNCTUATOR("<", PP_TOKEN_LESS),
    PUNCTUATOR(">", PP_TOKEN_GREATER),
    PUNCTUATOR("^", PP_TOKEN_CARET),
    PUNCTUATOR("|", PP_TOKEN_PIPE),
    PUNCTUATOR("?", PP_TOKEN_QUESTION),
    PUNCTUATOR(":", PP_TOKEN_COLON),
    PUNCTUATOR(";", PP_TOKEN_SEMICOLON),
    PUNCTUATOR("=", PP_TOKEN_ASSIGN),
    PUNCTUATOR(",", PP_TOKEN_COMMA),
    PUNCTUATOR("#", PP_TOKEN_HASH)
};

static size_t punctuator_list_size = sizeof(punctuators) / sizeof(punctuators[0]);

#undef PUNCTUATOR

static bool lexer_at_end(const Lexer *lexer) {
    return lexer->offset >= lexer->source->length;
}

static char lexer_peek(const Lexer *lexer, size_t lookahead) {
    size_t position = lexer->offset + lookahead;

    if (position >= lexer->source->length) {
        return '\0';
    }

    return lexer->source->contents[position];
}

static SourceLocation lexer_location(const Lexer *lexer) {
    return (SourceLocation){
        .file = lexer->source,
        .offset = lexer->offset,
        .line = lexer->line,
        .column = lexer->column
    };
}

static char lexer_advance(Lexer *lexer) {
    if (lexer_at_end(lexer)) {
        return '\0';
    }

    const char character = lexer->source->contents[lexer->offset++];

    if (character == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }

    return character;
}

static bool is_ascii_letter(const char character) {
    return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z');
}

static bool is_ascii_digit(const char character) {
    return (character >= '0' && character <= '9');
}

static bool graphic_chars[] = {
    ['_'] = true,
    ['{'] = true,
    ['}'] = true,
    ['['] = true,
    [']'] = true,
    ['#'] = true,
    ['('] = true,
    [')'] = true,
    ['<'] = true,
    ['>'] = true,
    ['%'] = true,
    [':'] = true,
    [';'] = true,
    ['.'] = true,
    ['?'] = true,
    ['*'] = true,
    ['+'] = true,
    ['-'] = true,
    ['/'] = true,
    ['^'] = true,
    ['&'] = true,
    ['|'] = true,
    ['~'] = true,
    ['!'] = true,
    ['='] = true,
    [','] = true,
    ['\\'] = true,
    ['"'] = true,
    ['\''] = true
};

static bool is_graphic_character(const char character) {
    return graphic_chars[(size_t)character];
}

static bool is_source_character(const char character) {
    return is_ascii_letter(character) ||
        is_ascii_digit(character) ||
        is_graphic_character(character);
}

static bool lexer_starts_ucn(const Lexer *lexer) {
    if (lexer_peek(lexer, 0) == '\\' &&
        (lexer_peek(lexer, 1) == 'u' || lexer_peek(lexer, 1) == 'U')
    ) {
        return true;
    }

    return false;
}

static bool is_ascii_identifier_start(const char character) {
    return is_ascii_letter(character) || character == '_';
}

static bool is_ascii_identifier_continue(const char character) {
    return is_ascii_identifier_start(character) ||
           is_ascii_digit(character);
}

static bool lexer_starts_identifier(const Lexer *lexer) {
    return is_ascii_identifier_start(lexer_peek(lexer, 0)) ||
           lexer_starts_ucn(lexer);
}

static bool is_horizontal_space(const char character) {
    switch (character) {
        case ' ':
        case '\t':
        case '\v':
        case '\f':
            return true;

        default:
            return false;
    }
}

static bool is_ascii_pp_number_continue(const char character) {
    return
        is_ascii_letter(character) ||
        is_ascii_digit(character) ||
        character == '_' ||
        character == '.';
}

static bool is_hexadecimal_digit(const char character) {
    return is_ascii_digit(character) ||
           (character >= 'a' && character <= 'f') ||
           (character >= 'A' && character <= 'F');
}

static uint32_t ascii_hex_value(const char character) {
    if (character >= '0' && character <= '9') {
        return (uint32_t)(character - '0');
    }

    if (character >= 'a' && character <= 'f') {
        return (uint32_t)(character - 'a' + 10);
    }

    return (uint32_t)(character - 'A' + 10);
}

static bool is_valid_c99_ucn_code_point(const uint32_t code_point) {
    if (
        code_point < 0x00A0 &&
        code_point != 0x0024 && // $
        code_point != 0x0040 && // @
        code_point != 0x0060    // `
    ) {
        return false;
    }

    if (
        code_point >= 0xD800 &&
        code_point <= 0xDFFF
    ) {
        return false;
    }

    return true;
}

typedef struct CodePointRange {
    uint32_t first;
    uint32_t last;
} CodePointRange;

#define RANGE(first, last) { first, last }
#define SINGLE(value)      { value, value }

static const CodePointRange c99_identifier_ucn_ranges[] = {
	SINGLE(0x00AA),
	SINGLE(0x00BA),
	RANGE(0x00C0, 0x00D6),
	RANGE(0x00D8, 0x00F6),
	RANGE(0x00F8, 0x01F5),
	RANGE(0x01FA, 0x0217),
	RANGE(0x0250, 0x02A8),
	RANGE(0x1E00, 0x1E9B),
	RANGE(0x1EA0, 0x1EF9),
	SINGLE(0x207F),
	SINGLE(0x0386),
	RANGE(0x0388, 0x038A),
	SINGLE(0x038C),
	RANGE(0x038E, 0x03A1),
	RANGE(0x03A3, 0x03CE),
	RANGE(0x03D0, 0x03D6),
	SINGLE(0x03DA),
	SINGLE(0x03DC),
	SINGLE(0x03DE),
	SINGLE(0x03E0),
	RANGE(0x03E2, 0x03F3),
	RANGE(0x1F00, 0x1F15),
	RANGE(0x1F18, 0x1F1D),
	RANGE(0x1F20, 0x1F45),
	RANGE(0x1F48, 0x1F4D),
	RANGE(0x1F50, 0x1F57),
	SINGLE(0x1F59),
	SINGLE(0x1F5B),
	SINGLE(0x1F5D),
	RANGE(0x1F5F, 0x1F7D),
	RANGE(0x1F80, 0x1FB4),
	RANGE(0x1FB6, 0x1FBC),
	RANGE(0x1FC2, 0x1FC4),
	RANGE(0x1FC6, 0x1FCC),
	RANGE(0x1FD0, 0x1FD3),
	RANGE(0x1FD6, 0x1FDB),
	RANGE(0x1FE0, 0x1FEC),
	RANGE(0x1FF2, 0x1FF4),
	RANGE(0x1FF6, 0x1FFC),
	RANGE(0x0401, 0x040C),
	RANGE(0x040E, 0x044F),
	RANGE(0x0451, 0x045C),
	RANGE(0x045E, 0x0481),
	RANGE(0x0490, 0x04C4),
	RANGE(0x04C7, 0x04C8),
	RANGE(0x04CB, 0x04CC),
	RANGE(0x04D0, 0x04EB),
	RANGE(0x04EE, 0x04F5),
	RANGE(0x04F8, 0x04F9),
	RANGE(0x0531, 0x0556),
	RANGE(0x0561, 0x0587),
	RANGE(0x05B0, 0x05B9),
	RANGE(0x05BB, 0x05BD),
	SINGLE(0x05BF),
	RANGE(0x05C1, 0x05C2),
	RANGE(0x05D0, 0x05EA),
	RANGE(0x05F0, 0x05F2),
	RANGE(0x0621, 0x063A),
	RANGE(0x0640, 0x0652),
	RANGE(0x0670, 0x06B7),
	RANGE(0x06BA, 0x06BE),
	RANGE(0x06C0, 0x06CE),
	RANGE(0x06D0, 0x06DC),
	RANGE(0x06E5, 0x06E8),
	RANGE(0x06EA, 0x06ED),
	RANGE(0x0901, 0x0903),
	RANGE(0x0905, 0x0939),
	RANGE(0x093E, 0x094D),
	RANGE(0x0950, 0x0952),
	RANGE(0x0958, 0x0963),
	RANGE(0x0981, 0x0983),
	RANGE(0x0985, 0x098C),
	RANGE(0x098F, 0x0990),
	RANGE(0x0993, 0x09A8),
	RANGE(0x09AA, 0x09B0),
	SINGLE(0x09B2),
	RANGE(0x09B6, 0x09B9),
	RANGE(0x09BE, 0x09C4),
	RANGE(0x09C7, 0x09C8),
	RANGE(0x09CB, 0x09CD),
	RANGE(0x09DC, 0x09DD),
	RANGE(0x09DF, 0x09E3),
	RANGE(0x09F0, 0x09F1),
	SINGLE(0x0A02),
	RANGE(0x0A05, 0x0A0A),
	RANGE(0x0A0F, 0x0A10),
	RANGE(0x0A13, 0x0A28),
	RANGE(0x0A2A, 0x0A30),
	RANGE(0x0A32, 0x0A33),
	RANGE(0x0A35, 0x0A36),
	RANGE(0x0A38, 0x0A39),
	RANGE(0x0A3E, 0x0A42),
	RANGE(0x0A47, 0x0A48),
	RANGE(0x0A4B, 0x0A4D),
	RANGE(0x0A59, 0x0A5C),
	SINGLE(0x0A5E),
	SINGLE(0x0A74),
	RANGE(0x0A81, 0x0A83),
	RANGE(0x0A85, 0x0A8B),
	SINGLE(0x0A8D),
	RANGE(0x0A8F, 0x0A91),
	RANGE(0x0A93, 0x0AA8),
	RANGE(0x0AAA, 0x0AB0),
	RANGE(0x0AB2, 0x0AB3),
	RANGE(0x0AB5, 0x0AB9),
	RANGE(0x0ABD, 0x0AC5),
	RANGE(0x0AC7, 0x0AC9),
	RANGE(0x0ACB, 0x0ACD),
	SINGLE(0x0AD0),
	SINGLE(0x0AE0),
	RANGE(0x0B01, 0x0B03),
	RANGE(0x0B05, 0x0B0C),
	RANGE(0x0B0F, 0x0B10),
	RANGE(0x0B13, 0x0B28),
	RANGE(0x0B2A, 0x0B30),
	RANGE(0x0B32, 0x0B33),
	RANGE(0x0B36, 0x0B39),
	RANGE(0x0B3E, 0x0B43),
	RANGE(0x0B47, 0x0B48),
	RANGE(0x0B4B, 0x0B4D),
	RANGE(0x0B5C, 0x0B5D),
	RANGE(0x0B5F, 0x0B61),
	RANGE(0x0B82, 0x0B83),
	RANGE(0x0B85, 0x0B8A),
	RANGE(0x0B8E, 0x0B90),
	RANGE(0x0B92, 0x0B95),
	RANGE(0x0B99, 0x0B9A),
	SINGLE(0x0B9C),
	RANGE(0x0B9E, 0x0B9F),
	RANGE(0x0BA3, 0x0BA4),
	RANGE(0x0BA8, 0x0BAA),
	RANGE(0x0BAE, 0x0BB5),
	RANGE(0x0BB7, 0x0BB9),
	RANGE(0x0BBE, 0x0BC2),
	RANGE(0x0BC6, 0x0BC8),
	RANGE(0x0BCA, 0x0BCD),
	RANGE(0x0C01, 0x0C03),
	RANGE(0x0C05, 0x0C0C),
	RANGE(0x0C0E, 0x0C10),
	RANGE(0x0C12, 0x0C28),
	RANGE(0x0C2A, 0x0C33),
	RANGE(0x0C35, 0x0C39),
	RANGE(0x0C3E, 0x0C44),
	RANGE(0x0C46, 0x0C48),
	RANGE(0x0C4A, 0x0C4D),
	RANGE(0x0C60, 0x0C61),
	RANGE(0x0C82, 0x0C83),
	RANGE(0x0C85, 0x0C8C),
	RANGE(0x0C8E, 0x0C90),
	RANGE(0x0C92, 0x0CA8),
	RANGE(0x0CAA, 0x0CB3),
	RANGE(0x0CB5, 0x0CB9),
	RANGE(0x0CBE, 0x0CC4),
	RANGE(0x0CC6, 0x0CC8),
	RANGE(0x0CCA, 0x0CCD),
	SINGLE(0x0CDE),
	RANGE(0x0CE0, 0x0CE1),
	RANGE(0x0D02, 0x0D03),
	RANGE(0x0D05, 0x0D0C),
	RANGE(0x0D0E, 0x0D10),
	RANGE(0x0D12, 0x0D28),
	RANGE(0x0D2A, 0x0D39),
	RANGE(0x0D3E, 0x0D43),
	RANGE(0x0D46, 0x0D48),
	RANGE(0x0D4A, 0x0D4D),
	RANGE(0x0D60, 0x0D61),
	RANGE(0x0E01, 0x0E3A),
	RANGE(0x0E40, 0x0E5B),
	RANGE(0x0E81, 0x0E82),
	SINGLE(0x0E84),
	RANGE(0x0E87, 0x0E88),
	SINGLE(0x0E8A),
	SINGLE(0x0E8D),
	RANGE(0x0E94, 0x0E97),
	RANGE(0x0E99, 0x0E9F),
	RANGE(0x0EA1, 0x0EA3),
	SINGLE(0x0EA5),
	SINGLE(0x0EA7),
	RANGE(0x0EAA, 0x0EAB),
	RANGE(0x0EAD, 0x0EAE),
	RANGE(0x0EB0, 0x0EB9),
	RANGE(0x0EBB, 0x0EBD),
	RANGE(0x0EC0, 0x0EC4),
	SINGLE(0x0EC6),
	RANGE(0x0EC8, 0x0ECD),
	RANGE(0x0EDC, 0x0EDD),
	SINGLE(0x0F00),
	RANGE(0x0F18, 0x0F19),
	SINGLE(0x0F35),
	SINGLE(0x0F37),
	SINGLE(0x0F39),
	RANGE(0x0F3E, 0x0F47),
	RANGE(0x0F49, 0x0F69),
	RANGE(0x0F71, 0x0F84),
	RANGE(0x0F86, 0x0F8B),
	RANGE(0x0F90, 0x0F95),
	SINGLE(0x0F97),
	RANGE(0x0F99, 0x0FAD),
	RANGE(0x0FB1, 0x0FB7),
	SINGLE(0x0FB9),
	RANGE(0x10A0, 0x10C5),
	RANGE(0x10D0, 0x10F6),
	RANGE(0x3041, 0x3093),
	RANGE(0x309B, 0x309C),
	RANGE(0x30A1, 0x30F6),
	RANGE(0x30FB, 0x30FC),
	RANGE(0x3105, 0x312C),
	RANGE(0x4E00, 0x9FA5),
	RANGE(0xAC00, 0xD7A3),
	SINGLE(0x00B5),
	SINGLE(0x00B7),
	RANGE(0x02B0, 0x02B8),
	SINGLE(0x02BB),
	RANGE(0x02BD, 0x02C1),
	RANGE(0x02D0, 0x02D1),
	RANGE(0x02E0, 0x02E4),
	SINGLE(0x037A),
	SINGLE(0x0559),
	SINGLE(0x093D),
	SINGLE(0x0B3D),
	SINGLE(0x1FBE),
	RANGE(0x203F, 0x2040),
	SINGLE(0x2102),
	SINGLE(0x2107),
	RANGE(0x210A, 0x2113),
	SINGLE(0x2115),
	RANGE(0x2118, 0x211D),
	SINGLE(0x2124),
	SINGLE(0x2126),
	SINGLE(0x2128),
	RANGE(0x212A, 0x2131),
	RANGE(0x2133, 0x2138),
	RANGE(0x2160, 0x2182),
	RANGE(0x3005, 0x3007),
	RANGE(0x3021, 0x3029)
};

static const CodePointRange c99_identifier_digit_ranges[] = {
    RANGE(0x0660, 0x0669),
    RANGE(0x06F0, 0x06F9),
    RANGE(0x0966, 0x096F),
    RANGE(0x09E6, 0x09EF),
    RANGE(0x0A66, 0x0A6F),
    RANGE(0x0AE6, 0x0AEF),
    RANGE(0x0B66, 0x0B6F),
    RANGE(0x0BE7, 0x0BEF),
    RANGE(0x0C66, 0x0C6F),
    RANGE(0x0CE6, 0x0CEF),
    RANGE(0x0D66, 0x0D6F),
    RANGE(0x0E50, 0x0E59),
    RANGE(0x0ED0, 0x0ED9),
    RANGE(0x0F20, 0x0F33)
};

static bool code_point_in_ranges(
    const uint32_t code_point,
    const CodePointRange *ranges,
    const size_t count
) {
    size_t low = 0;
    size_t high = count;

    while (low < high) {
        const size_t middle = low + (high - low) / 2;
        const CodePointRange range = ranges[middle];

        if (code_point < range.first) {
            high = middle;
        } else if (code_point > range.last) {
            low = middle + 1;
        } else {
            return true;
        }
    }

    return false;
}

static bool is_valid_c99_identifier_ucn(const uint32_t code_point, const bool is_initial) {
    if (!code_point_in_ranges(
            code_point,
            c99_identifier_ucn_ranges,
            _countof(c99_identifier_ucn_ranges)
        )) {
        return false;
    }

        if (
        is_initial &&
        code_point_in_ranges(
            code_point,
            c99_identifier_digit_ranges,
            _countof(c99_identifier_digit_ranges)
        )
    ) {
            return false;
    }

    return true;
}

static ErrorCode lexer_consume_ucn(Lexer *lexer, uint32_t *result, LexerError *error) {
    if (!lexer_starts_ucn(lexer)) {
        if (error != nullptr) {
            *error = (LexerError){
                .span = {
                    .begin = lexer_location(lexer),
                    .end = lexer_location(lexer)
                },
                .message = strdup("Not a UCN")
            };
        }

        return ERR_OK;
    }

    const SourceLocation begin = lexer_location(lexer);

    lexer_advance(lexer); // "\"

    const char c = lexer_advance(lexer); // "u" or "U"

    const size_t digit_count = c == 'u' ? 4 : 8;

    uint32_t code_point = 0;

    for (size_t i = 0; i < digit_count; ++i) {
        const char digit = lexer_peek(lexer, 0);

        if (!is_hexadecimal_digit(digit)) {
            if (error != nullptr) {
                *error = (LexerError){
                    .span = {
                        .begin = begin,
                        .end = lexer_location(lexer)
                    },
                };

                char message[256];

                if (c == 'u') {
                    snprintf(message, sizeof(message), "\\u must be followed by exactly 4 hexadecimal digits");
                } else {
                    snprintf(message, sizeof(message), "\\U must be followed by exactly 8 hexadecimal digits");
                }

                error->message = strdup(message);
            }

            return ERR_INTERNAL;
        }

        code_point =
            (code_point << 4) |
            ascii_hex_value(digit);

        lexer_advance(lexer);
    }

    if (!is_valid_c99_ucn_code_point(code_point)) {
        if (error != nullptr) {
            *error = (LexerError){
                .span = {
                    .begin = begin,
                    .end = lexer_location(lexer)
                },
                .message = strdup("universal character name designates a forbidden character")
            };
        }

        return ERR_INTERNAL;
    }

    if (result != nullptr) {
        *result = code_point;
    }

    return ERR_OK;
}

void lexer_init(Lexer *lexer, const SourceFile *source) {
    *lexer = (Lexer){
        .source = source,
        .offset = 0,
        .line = 1,
        .column = 1,
        .start_of_line = true,
        .pending_space = false,
        .inside_block_comment = false
    };
}

static bool lexer_matches(const Lexer *lexer, const char *text, const size_t length) {
    for (size_t i = 0; i < length; ++i) {
        if (lexer_peek(lexer, i) != text[i]) {
            return false;
        }
    }

    return true;
}

static PPToken make_token(
    Lexer *lexer,
    const PPTokenKind kind,
    const SourceLocation *begin,
    const bool leading_space,
    const bool start_of_line
) {
    PPToken token = {
        .kind = kind,
        .actual_span = {
            .begin = *begin,
            .end = lexer_location(lexer)
        },
        .presumed_span = {
            .begin = *begin,
            .end = lexer_location(lexer)
        },
        .leading_space = leading_space,
        .start_of_line = start_of_line
    };

    memset(&token.data, 0, sizeof(token.data));

    if (kind == PP_TOKEN_NEWLINE) {
        lexer->start_of_line = true;
    } else if (kind != PP_TOKEN_EOF) {
        lexer->start_of_line = false;
    }

    lexer->pending_space = false;

    return token;
}

ErrorCode lex_punctuator(
    Lexer *lexer,
    PPToken *result,
    const SourceLocation begin,
    const bool leading_space,
    const bool start_of_line
) {
    for (size_t i = 0; i < punctuator_list_size; i++) {
        const PunctuatorEntry *entry = &punctuators[i];

        if (!lexer_matches(lexer, entry->spelling, entry->length)) {
            continue;
        }

        for (size_t j = 0; j < entry->length; j++) {
            lexer_advance(lexer);
        }

        *result = make_token(
            lexer,
            entry->kind,
            &begin,
            leading_space,
            start_of_line
        );

        return ERR_OK;
    }

    return ERR_INTERNAL;
}

static ErrorCode lex_quoted_token(
    Lexer *lexer,
    PPToken *result,
    LexerError *error,
    const SourceLocation begin,
    const bool leading_space,
    const bool start_of_line,
    const bool wide,
    const char quote
) {
    if (wide) {
        lexer_advance(lexer); // L
    }

    lexer_advance(lexer); // opening quote

    bool has_character = false;

    while (!lexer_at_end(lexer)) {
        const char current = lexer_peek(lexer, 0);

        if (current == '\n') {
            if (error != nullptr) {
                char literal_kind[256] = "character literal\0";
                char message[256];
                if (quote == '"') {
                    sprintf(literal_kind, "string literal");
                }

                *error = (LexerError){
                    .span = {
                        .begin = begin,
                        .end = lexer_location(lexer)
                    }
                };

                snprintf(
                    message,
                    sizeof(message),
                    "Could not find %c to end %s",
                    quote,
                    literal_kind
                );

                error->message = strdup(message);
            }

            return ERR_INTERNAL;
        }

        if (current == quote) {
            lexer_advance(lexer);

            /*
             * Character constants require at least one c-char.
             * Empty strings are valid.
             */
            if (quote == '\'' && !has_character) {
                if (error != nullptr) {
                    *error = (LexerError){
                        .span = {
                            .begin = begin,
                            .end = lexer_location(lexer)
                        },
                        .message = strdup("character literal needs a character in the quotation marks")
                    };
                }

                return ERR_INTERNAL;
            }

            *result = make_token(
                lexer,
                quote == '\''
                    ? PP_TOKEN_CHARACTER_CONSTANT
                    : PP_TOKEN_STRING_LITERAL,
                &begin,
                leading_space,
                start_of_line
            );

            result->data.string = copy_string(&result->actual_span);

            result->wide = wide;
            return ERR_OK;
        }


        if (current == '\\') {
            lexer_advance(lexer); // backslash

            if (lexer_at_end(lexer)) {
                if (error != nullptr) {
                    *error = (LexerError){
                        .span = {
                            .begin = begin,
                            .end = lexer_location(lexer)
                        },
                        .message = strdup("escape character incomplete while at end of file")
                    };
                }

                return ERR_INTERNAL;
            }

            /*
             * Translation-phase line splicing should already have removed
             * backslash-newline pairs. Seeing one here is therefore invalid.
             */
            if (lexer_peek(lexer, 0) == '\n') {
                if (error != nullptr) {
                    *error = (LexerError){
                        .span = {
                            .begin = begin,
                            .end = lexer_location(lexer)
                        },
                        .message = strdup("escape character incomplete while at newline")
                    };
                }

                return ERR_INTERNAL;
            }

            lexer_advance(lexer); // first character of escape sequence
            has_character = true;
            continue;
        }

        lexer_advance(lexer);
        has_character = true;
    }

    if (error != nullptr) {
        char literal_kind[256] = "character literal\0";
        char message[256];
        if (quote == '"') {
            sprintf(literal_kind, "string literal");
        }

        *error = (LexerError){
            .span = {
                .begin = begin,
                .end = lexer_location(lexer)
            }
        };

        snprintf(
            message,
            sizeof(message),
            "incomplete %s",
            literal_kind
        );

        error->message = strdup(message);
    }

    return ERR_INTERNAL;
}

static ErrorCode lex_pp_number(
    Lexer *lexer,
    PPToken *result,
    LexerError *error,
    const SourceLocation *begin,
    const bool leading_space,
    const bool start_of_line
) {
    char previous = '\0';

    while (!lexer_at_end(lexer)) {
        if (lexer_starts_ucn(lexer)) {
            uint32_t code_point;

            const ErrorCode code = lexer_consume_ucn(lexer, &code_point, error);

            if (code != ERR_OK) return code;

            /*
             * The UCN interrupted any literal e/E/p/P + sign sequence.
             */
            previous = '\0';
            continue;
        }

        const char current = lexer_peek(lexer, 0);

        if (is_ascii_pp_number_continue(current)) {
            previous = lexer_advance(lexer);
            continue;
        }

        if (
            (current == '+' || current == '-') &&
            (
                previous == 'e' ||
                previous == 'E' ||
                previous == 'p' ||
                previous == 'P'
            )
        ) {
            previous = lexer_advance(lexer);
            continue;
        }

        break;
    }

    *result = make_token(
        lexer,
        PP_TOKEN_NUMBER,
        begin,
        leading_space,
        start_of_line
    );

    return ERR_OK;
}

static ErrorCode lex_identifier(
    Lexer *lexer,
    PPToken *result,
    LexerError *error,
    const SourceLocation *begin,
    const bool leading_space,
    const bool start_of_line
) {
    bool first = true;

    while (!lexer_at_end(lexer)) {
        if (lexer_starts_ucn(lexer)) {
            uint32_t code_point;
            const SourceLocation ucn_start = lexer_location(lexer);

            const ErrorCode code = lexer_consume_ucn(lexer, &code_point, error);

            if (code != ERR_OK) {
                return code;
            }

            if (!is_valid_c99_identifier_ucn(code_point, first)) {
                if (error != nullptr) {
                    *error = (LexerError){
                        .span = {
                            .begin = ucn_start,
                            .end = lexer_location(lexer)
                        },
                        .message = strdup("invalid UCN")
                    };
                }

                return ERR_INTERNAL;
            }

            first = false;
            continue;
        }

        const char current = lexer_peek(lexer, 0);

        if (
            first
                ? is_ascii_identifier_start(current)
                : is_ascii_identifier_continue(current)
        ) {
            lexer_advance(lexer);
            first = false;
            continue;
        }

        break;
    }

    *result = make_token(
        lexer,
        PP_TOKEN_IDENTIFIER,
        begin,
        leading_space,
        start_of_line
    );

    result->data.string = copy_string(&result->actual_span);

    return ERR_OK;
}

ErrorCode lexer_next(Lexer *lexer, PPToken *token, LexerError *error) {
    for (;;) {
        if (lexer_at_end(lexer)) {
            const SourceLocation location = lexer_location(lexer);

            if (lexer->inside_block_comment) {
                if (error != nullptr) {
                    *error = (LexerError){
                        .span = {
                            .begin = lexer->block_comment_start,
                            .end = location
                        },
                        .message = strdup("unterminated block comment")
                    };
                }

                return false;
            }

            *token = make_token(
                lexer,
                PP_TOKEN_EOF,
                &location,
                lexer->pending_space,
                lexer->start_of_line
            );

            return ERR_OK;
        }

        const char current = lexer_peek(lexer, 0);

        if (is_horizontal_space(current)) {
            lexer->pending_space = true;
            lexer_advance(lexer);
            continue;
        }

        if (current == '\n') {
            const SourceLocation begin = lexer_location(lexer);

            lexer_advance(lexer);

            *token = make_token(
                lexer,
                PP_TOKEN_NEWLINE,
                &begin,
                lexer->pending_space,
                lexer->start_of_line
            );

            return ERR_OK;
        }

        if (lexer->inside_block_comment) {
            lexer_advance(lexer);
            if (current == '*') {
                if (lexer_peek(lexer, 0) == '/') {
                    lexer->inside_block_comment = false;
                    lexer->pending_space = true;
                    lexer_advance(lexer);
                }
            }

            continue;
        }

        const SourceLocation begin = lexer_location(lexer);

        const bool leading_space = lexer->pending_space;
        const bool start_of_line = lexer->start_of_line;

        if (current == '/' && lexer_peek(lexer, 1) == '/') {
            lexer_advance(lexer); // "/"
            lexer_advance(lexer); // "/"

            while (!lexer_at_end(lexer) && lexer_peek(lexer, 0) != '\n') {
                lexer_advance(lexer);
            }

            lexer->pending_space = true;
            continue;
        }

        if (current == '/' && lexer_peek(lexer, 1) == '*') {
            lexer_advance(lexer); // "/"
            lexer_advance(lexer); // "*"

            lexer->inside_block_comment = true;
            lexer->block_comment_start = lexer_location(lexer);
            continue;
        }

        if (
            current == '"' ||
            current == '\'' ||
            (
                current == 'L' &&
                (
                    lexer_peek(lexer, 1) == '"' ||
                    lexer_peek(lexer, 1) == '\''
                )
            )
        ) {
            const bool wide = current == 'L' ? true : false;
            char quote = current;
            if (wide) quote = lexer_peek(lexer, 1);

            return lex_quoted_token(
                lexer,
                token,
                error,
                lexer_location(lexer),
                leading_space,
                start_of_line,
                wide,
                quote
            );
        }

        if (lexer_starts_identifier(lexer)) {
            return lex_identifier(
                lexer,
                token,
                error,
                &begin,
                leading_space,
                start_of_line
            );
        }

        if (
            is_ascii_digit(current) ||
            (current == '.' &&is_ascii_digit(lexer_peek(lexer, 1)))
        ) {
            return lex_pp_number(
                lexer,
                token,
                error,
                &begin,
                leading_space,
                start_of_line
            );
        }

        if (lex_punctuator(lexer, token, lexer_location(lexer), leading_space, start_of_line) == ERR_OK) {
            return ERR_OK;
        }

        lexer_advance(lexer);

        *token = make_token(
            lexer,
            PP_TOKEN_OTHER_CHARACTER,
            &begin,
            leading_space,
            start_of_line
        );

        return ERR_OK;
    }
}

ErrorCode lexer_next_header_name(Lexer *lexer, PPToken *token, LexerError *error, const bool h_char) {
    bool has_char = false;
    const SourceLocation begin = lexer_location(lexer);

    const bool leading_space = lexer->pending_space;
    const bool start_of_line = lexer->start_of_line;

    if (h_char) {
        while (lexer_peek(lexer, 0) != '\n' ||
            lexer_peek(lexer, 0) != '>' ||
            is_source_character(lexer_peek(lexer, 0))
        ) {
            has_char = true;
            lexer_advance(lexer);
        }
    } else {
        while (lexer_peek(lexer, 0) != '\n' ||
            lexer_peek(lexer, 0) != '"' ||
            is_source_character(lexer_peek(lexer, 0))
        ) {
            has_char = true;
            lexer_advance(lexer);
        }
    }

    if (!has_char) {
        if (error != nullptr) {
            *error = (LexerError){
                .span = {
                    .begin = begin,
                    .end = lexer_location(lexer),
                },
                .message = strdup("header path is empty")
            };
        }

        return ERR_INTERNAL;
    }

    *token = make_token(
        lexer,
        PP_TOKEN_HEADER_NAME,
        &begin,
        leading_space,
        start_of_line
    );

    return ERR_OK;
}
