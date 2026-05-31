#include "lexer.h"
#include "../lib/error.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

u64 advance(Position* pos, const u8 *string) {
    pos->column++;
    if (string[pos->idx] == '\n') {
        pos->line++;
        pos->column = 1;
    }
    pos->idx++;
    return pos->idx;
}

void init_TokenList(TokenList* list) {
    if (!list) return;

    list->count = 0;
    list->size = 8;

    list->tokens = malloc(sizeof(Token) * list->size);
    if (!list->tokens) {
        exit(1);
    }
}

void free_TokenList(TokenList* list) {
    if (!list) return;

    free(list->tokens);
}

void token_push(TokenList* list, Token token) {
    if (!list) return;

    if (list->count+1 >= list->size) {
        list->size *= 2;
        Token *tmp = realloc(list->tokens, list->size * sizeof(Token));
        if (!tmp) {
            exit(1);
        }
        list->tokens = tmp;
    }

    list->tokens[list->count++] = token;
}

i64 parse_num(u8 *string, Position *pos, const u64 *idx) {
    u64 buf_capacity = 16;
    u64 buf_len = 0;
    u8 *buff = malloc(buf_capacity * sizeof(u8));

    while (isdigit(string[*idx])) {
        if (buf_len + 1 >= buf_capacity) {
            buf_capacity *= 2;
            void *tmp = realloc(buff, buf_capacity * sizeof(u8));
            if (!tmp) exit(1);
            buff = tmp;
        }
        buff[buf_len++] = string[advance(pos, string)-1];
    }
    buff[buf_len] = '\0';

    return _atoi64((char *)buff);
}

void tokenise(TokenList *list, u8 *filename, u8 *string) {
    if (!list) return;

    Position pos;
    pos.filename = filename;
    pos.source = string;
    pos.idx = 0;
    pos.line = 1;
    pos.column = 1;
    const u64 *i = &pos.idx;

    init_TokenList(list);

    while (string[*i] != '\0') {
        Token t;
        t.value = NULL;

        if (isspace(string[*i])) {
            if (string[*i] == '\n') {
                t.type = TT_NEWLINE;
                t.pos = pos;
                token_push(list, t);
            }
            advance(&pos, string);
            continue;
        }

        if (string[*i] == ';') {
            do {
                advance(&pos, string);
            } while (string[*i] != '\n');
            continue;
        }

        if (string[*i] == ':') {
            t.pos = pos;
            t.type = TT_COLON;
            token_push(list, t);
            advance(&pos, string);
            continue;
        }

        if (string[*i] == '=') {
            t.pos = pos;
            t.type = TT_EQUALS;
            token_push(list, t);
            advance(&pos, string);
            continue;
        }

        if (string[*i] == '+') {
            t.pos = pos;
            t.type = TT_PLUS;
            token_push(list, t);
            advance(&pos, string);
            continue;
        }

        if (string[*i] == '-') {
            t.pos = pos;
            t.type = TT_MINUS;
            token_push(list, t);
            advance(&pos, string);
            continue;
        }

        if (string[*i] == ',') {
            t.pos = pos;
            t.type = TT_COMMA;
            token_push(list, t);
            advance(&pos, string);
            continue;
        }

        if (string[*i] == '[') {
            t.pos = pos;
            t.type = TT_L_SQUARE_BRACKET;
            token_push(list, t);
            advance(&pos, string);
            continue;
        }

        if (string[*i] == ']') {
            t.pos = pos;
            t.type = TT_R_SQUARE_BRACKET;
            token_push(list, t);
            advance(&pos, string);
            continue;
        }

        if (isdigit(string[*i])) {
            t.pos = pos;
            t.type = TT_IMMEDIATE;
            i64 num = parse_num(string, &pos, i);
            t.value = malloc(sizeof(i64));
            memcpy(t.value, &num, sizeof(i64));
            token_push(list, t);
            continue;
        }

        if (isalpha(string[*i]) || string[*i] == '_' || string[*i] == '.') {
            t.pos = pos;

            u64 buf_capacity = 16;
            u64 buf_len = 0;
            u8 *buff = malloc(buf_capacity * sizeof(u8));

            while (isalnum(string[*i]) || string[*i] == '_' || string[*i] == '.') {
                if (buf_len + 1 >= buf_capacity) {
                    buf_capacity *= 2;
                    void *tmp = realloc(buff, buf_capacity * sizeof(u8));
                    if (!tmp) exit(1);
                    buff = tmp;
                }
                buff[buf_len++] = string[advance(&pos, string)-1];
            }
            buff[buf_len] = '\0';

            if (ismnemonic(buff)) {
                t.type = TT_MNEMONIC;
                t.value = strdup((char *)buff);
            } else if (isregister(buff)) {
                t.type = TT_REGISTER;
                t.value = strdup((char *)buff);
            } else if (issizespec(buff)) {
                t.type = TT_SIZESPEC;
                t.value = strdup((char *)buff);
            } else if (buff[0] == '.') { // its a directive
                t.type = TT_DIRECTIVE;
                t.value = strdup((char *)buff);
            } else {
                t.type = TT_IDENTIFIER;
                t.value = strdup((char *)buff);
            }

            token_push(list, t);
            continue;
        }

        advance(&pos, string);
    }

    Token t;
    t.value = nullptr;
    t.pos = pos;
    t.type = TT_EOF;
    token_push(list, t);
}
