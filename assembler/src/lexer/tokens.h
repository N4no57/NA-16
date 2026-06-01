#ifndef NA_16_TOKENS_H
#define NA_16_TOKENS_H

#include "../lib/asmlib.h"

typedef enum TokenType {
    TT_MNEMONIC,
    TT_REGISTER,
    TT_IMMEDIATE,
    TT_SIZESPEC,
    TT_IDENTIFIER,
    TT_DIRECTIVE,
    TT_STRING,
    TT_COMMA,
    TT_COLON,
    TT_EQUALS,
    TT_PLUS,
    TT_MINUS,
    TT_L_SQUARE_BRACKET,
    TT_R_SQUARE_BRACKET,
    TT_NEWLINE,
    TT_EOF
} TokenType;

typedef struct Token {
    TokenType type;
    Position pos;
    void *value;
} Token;

typedef struct TokenList {
    u64 count;
    u64 size;
    Token *tokens;
} TokenList;

void init_TokenList(TokenList* list);
void free_TokenList(TokenList* list);

void token_push(TokenList* list, const Token *token);
void token_insert(TokenList* list, const Token *token, u64 idx);

void token_delete(TokenList* list, u64 idx);

#endif //NA_16_TOKENS_H
