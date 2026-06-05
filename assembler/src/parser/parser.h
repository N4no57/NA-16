#ifndef NA_16_PARSER_H
#define NA_16_PARSER_H

#include "../lexer/lexer.h"
#include "ast.h"

void consume_tok(const TokenList *tokens, u64 *idx, Token *tok);

void parse(NodeProgram *ast, TokenList *tokens);

#endif //NA_16_PARSER_H
