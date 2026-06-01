#ifndef NA_16_LEXER_H
#define NA_16_LEXER_H

#include "../lib/asmlib.h"
#include "tokens.h"

void tokenise(TokenList *list, u8 *filename, u8 *string);

#endif //NA_16_LEXER_H
