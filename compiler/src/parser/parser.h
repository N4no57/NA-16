#ifndef NA_16_PARSER_H
#define NA_16_PARSER_H

#include "../lexer/source.h"
#include "token_stream.h"
#include "ast.h"

typedef struct ParserError {
    SourceSpan span;
    const char *message;
} ParserError;

typedef struct Parser {
    TokenStream *tokens;

    ParserError error;
} Parser;

void parser_init(Parser *parser, TokenStream *tokens);

Error parser_parse_translation_unit(Parser *parser, TranslationUnit *unit);

#endif //NA_16_PARSER_H
