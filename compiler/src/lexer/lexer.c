#include "lexer.h"

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

    char character = lexer->source->contents[lexer->offset++];

    if (character == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }

    return character;
}

static bool is_ascii_letter(char character) {
    return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z');
}

static bool is_ascii_digit(char character) {
    return (character >= '0' && character <= '9');
}

static bool is_identifier_start(char character) {
    return is_ascii_letter(character) || character == '_';
}

static bool is_identifier_continue(char character) {
    return is_identifier_start(character) || is_ascii_digit(character);
}

static bool is_horizontal_space(char character) {
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

static bool is_pp_number_continue(char character) {
    return
            is_ascii_letter(character) ||
            is_ascii_digit(character) ||
            character == '_' ||
            character == '.';
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
    const PPToken token = {
        .kind = kind,
        .span = {
            .begin = *begin,
            .end = lexer_location(lexer)
        },
        .leading_space = leading_space,
        .start_of_line = start_of_line
    };

    if (kind == PP_TOKEN_NEWLINE) {
        lexer->start_of_line = true;
    } else if (kind != PP_TOKEN_EOF) {
        lexer->start_of_line = false;
    }

    lexer->pending_space = false;

    return token;
}

bool lex_punctuator(Lexer *lexer, PPToken *result, SourceLocation begin, bool leading_space, bool start_of_line) {
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

        return true;
    }

    return false;
}

static PPToken lex_pp_number(
    Lexer *lexer,
    const SourceLocation *begin,
    const bool leading_space,
    const bool start_of_line
) {
    char previous = '\0';

    while (!lexer_at_end(lexer)) {
        const char current = lexer_peek(lexer, 0);

        if (is_pp_number_continue(current)) {
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

    return make_token(
        lexer,
        PP_TOKEN_NUMBER,
        begin,
        leading_space,
        start_of_line
    );
}

static PPToken lex_identifier(
    Lexer *lexer,
    const SourceLocation *begin,
    const bool leading_space,
    const bool start_of_line
) {
    lexer_advance(lexer);

    while (is_identifier_continue(lexer_peek(lexer, 0))) {
        lexer_advance(lexer);
    }

    return make_token(
        lexer,
        PP_TOKEN_IDENTIFIER,
        begin,
        leading_space,
        start_of_line
    );
}

bool lexer_next(Lexer *lexer, PPToken *token, LexerError *error) {
    for (;;) {
        if (lexer_at_end(lexer)) {
            const SourceLocation location = lexer_location(lexer);

            if (lexer->inside_block_comment) {
                *error = (LexerError){
                    .span = {
                        .begin = lexer->block_comment_start,
                        .end = location
                    },
                    .message = "unterminated block comment"
                };

                return false;
            }

            *token = make_token(
                lexer,
                PP_TOKEN_EOF,
                &location,
                lexer->pending_space,
                lexer->start_of_line
            );

            return true;
        }

        char current = lexer_peek(lexer, 0);

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

            return true;
        }

        if (lexer->inside_block_comment) {
            lexer_advance(lexer);
            if (current == '*') {
                if (lexer_peek(lexer, 0) == '/') {
                    lexer->inside_block_comment = false;
                    lexer_advance(lexer);
                }
            }

            continue;
        }

        const SourceLocation begin = lexer_location(lexer);

        const bool leading_space = lexer->pending_space;
        const bool start_of_line = lexer->start_of_line;

        if (is_identifier_start(current)) {
            *token = lex_identifier(
                lexer,
                &begin,
                leading_space,
                start_of_line
            );

            return true;
        }

        if (current == '/' && lexer_peek(lexer, 1) == '/') {
            while (!lexer_at_end(lexer) && lexer_peek(lexer, 0) != '\0') {
                lexer_advance(lexer);
            }

            lexer->pending_space = true;

            continue;
        }

        if (current == '/' && lexer_peek(lexer, 1) == '*') {
            lexer_advance(lexer);
            lexer->inside_block_comment = true;
            lexer->block_comment_start = lexer_location(lexer);
            continue;
        }

        if (
            is_ascii_digit(current) ||
            (current == '.' &&is_ascii_digit(lexer_peek(lexer, 1)))
        ) {
            *token = lex_pp_number(
                lexer,
                &begin,
                leading_space,
                start_of_line
            );

            return true;
        }

        lexer_advance(lexer);

        if (lex_punctuator(lexer, token, lexer_location(lexer), leading_space, start_of_line)) {
            return true;
        }

        make_token(
            lexer,
            PP_TOKEN_OTHER_CHARACTER,
            &begin,
            leading_space,
            start_of_line
        );

        return true;
    }
}
