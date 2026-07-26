#include "lexer.h"

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
    };
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

static PPToken lex_pp_number(
    Lexer *lexer,
    SourceLocation *begin,
    const bool leading_space,
    const bool start_of_line
) {
    char previous = '\0';

    while (!lexer_at_end(lexer)) {
        char current = lexer_peek(lexer, 0);

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
    SourceLocation *begin,
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
            SourceLocation location = lexer_location(lexer);

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
            SourceLocation begin = lexer_location(lexer);

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

        SourceLocation begin = lexer_location(lexer);

        bool leading_space = lexer->pending_space;
        bool start_of_line = lexer->start_of_line;

        if (is_identifier_start(current)) {
            *token = lex_identifier(
                lexer,
                &begin,
                leading_space,
                start_of_line
            );

            return true;
        }

        if (
            is_ascii_digit(current) ||
            (
                current == '.' &&
                is_ascii_digit(
                    lexer_peek(lexer, 1)
                )
            )
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

        switch (current) {
            case '(':
                *token = make_token(
                    lexer,
                    PP_TOKEN_LEFT_PAREN,
                    &begin,
                    leading_space,
                    start_of_line
                );
                return true;

            case ')':
                *token = make_token(
                    lexer,
                    PP_TOKEN_RIGHT_PAREN,
                    &begin,
                    leading_space,
                    start_of_line
                );
                return true;

            case '{':
                *token = make_token(
                    lexer,
                    PP_TOKEN_LEFT_BRACE,
                    &begin,
                    leading_space,
                    start_of_line
                );
                return true;

            case '}':
                *token = make_token(
                    lexer,
                    PP_TOKEN_RIGHT_BRACE,
                    &begin,
                    leading_space,
                    start_of_line
                );
                return true;

            case ';':
                *token = make_token(
                    lexer,
                    PP_TOKEN_SEMICOLON,
                    &begin,
                    leading_space,
                    start_of_line
                );
                return true;

            case '/':
                if (lexer_peek(lexer, 0) == '/') {
                    while (current != '\n' && current != '\0') {
                        lexer_advance(lexer);
                        current = lexer_peek(lexer, 0);
                    }

                    lexer->pending_space = true;
                } else if (lexer_peek(lexer, 0) == '*') {
                    lexer->inside_block_comment = true;
                }

                continue;

            case '*':
                if (lexer_peek(lexer, 1) == '/') {
                    lexer->inside_block_comment = false;
                    continue;
                }

                return false; // TODO

            default:
                if (error != NULL) {
                    error->span.begin = begin;
                    error->span.end =
                        lexer_location(lexer);
                    error->message =
                        "invalid character in source file";
                }

                return false;
        }
    }
}
