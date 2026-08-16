#ifndef NA_16_PARSER_H
#define NA_16_PARSER_H

#include "token_stream.h"
#include "ast.h"

typedef PreprocessorError ParserError;

typedef struct Parser {
    TokenStream *tokens;

    Vector errors;
} Parser;

void parser_init(Parser *parser, TokenStream *tokens);

Error parse_translation_unit(Parser *parser, TranslationUnit *unit);

#endif //NA_16_PARSER_H
