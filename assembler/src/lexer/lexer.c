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

bool ishexdigit(const char c) {
    if (isdigit(c)) return true;
    if (c >= 'A' && c <= 'F') return true;
    if (c >= 'a' && c <= 'f') return true;

    return false;
}

int detect_base(char **s) {
    const char *p = *s;

    if (p[0] == '0') {
        if (p[1] == 'x' || p[1] == 'X') {
            *s += 2;
            return 16;
        }
        if (p[1] == 'b' || p[1] == 'B') {
            *s += 2;
            return 2;
        }
        // leading 0 → octal (if you want C behavior)
        if (isdigit((unsigned char)p[1])) {
            *s += 1;
            return 8;
        }
    }

    return 10;
}

i64 parse_num(u8 *string, Position *pos, const u64 *idx) {
    u64 buf_capacity = 16;
    u64 buf_len = 0;
    char *buff = malloc(buf_capacity * sizeof(u8));

    while (ishexdigit((char)string[*idx]) || string[*idx] == 'x' || string[*idx] == 'b') {
        if (buf_len + 1 >= buf_capacity) {
            buf_capacity *= 2;
            void *tmp = realloc(buff, buf_capacity * sizeof(u8));
            if (!tmp) exit(1);
            buff = tmp;
        }
        buff[buf_len++] = (char)string[advance(pos, string)-1];
    }
    buff[buf_len] = '\0';

    const i32 radix = detect_base(&buff);

    char *end;

    const i64 num = strtoll(buff, &end, radix);

    if (end == buff) {
        error(*pos, "strtoll() failed: no characters");
    }

    if (*end != '\0') {
        error(*pos, "strtoll() failed: trailing junk \"%s\"", end);
    }

    return num;
}

void parse_string(TokenList *list, u8 *string, Position *pos, const u64 *idx) {
    Token t;
    t.pos = *pos;

    u64 buf_capacity = 16;
    u64 buf_len = 0;
    char *buff = malloc(buf_capacity * sizeof(char));

    while (string[*idx] != '"' && string[*idx] != '\0' && string[*idx] != '\n') {
        if (buf_len >= buf_capacity) {
            buf_capacity *= 2;
            char *tmp = realloc(buff, buf_capacity * sizeof(char));
            if (!tmp) exit(1);
            buff = tmp;
        }

        if (string[*idx] == '\\') {
            advance(pos, string);
            if (string[*idx] == '\\') {
                buff[buf_len++] = '\\';
            } else if (string[*idx] == '"') {
                buff[buf_len++] = '"';
            } else if (string[*idx] == 'n') {
                buff[buf_len++] = '\n';
            } else if (string[*idx] == 't') {
                buff[buf_len++] = '\t';
            } else if (string[*idx] == '0') {
                buff[buf_len++] = '\0';
            }

            advance(pos, string);
            continue;
        }

        buff[buf_len++] = (char)string[advance(pos, string)-1];
    }
    advance(pos, string); // consume '"'
    buff[buf_len] = '\0';

    String *s = malloc(sizeof(String));

    s->size = buf_len;
    s->str = malloc(s->size);

    memcpy(s->str, buff, s->size);

    t.type = TT_STRING;
    t.value = s;

    token_push(list, &t);

    free(buff);
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
                token_push(list, &t);
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
            token_push(list, &t);
            advance(&pos, string);
            continue;
        }

        if (string[*i] == '=') {
            t.pos = pos;
            t.type = TT_EQUALS;
            token_push(list, &t);
            advance(&pos, string);
            continue;
        }

        if (string[*i] == '+') {
            t.pos = pos;
            t.type = TT_PLUS;
            token_push(list, &t);
            advance(&pos, string);
            continue;
        }

        if (string[*i] == '-') {
            t.pos = pos;
            t.type = TT_MINUS;
            token_push(list, &t);
            advance(&pos, string);
            continue;
        }

        if (string[*i] == ',') {
            t.pos = pos;
            t.type = TT_COMMA;
            token_push(list, &t);
            advance(&pos, string);
            continue;
        }

        if (string[*i] == '[') {
            t.pos = pos;
            t.type = TT_L_SQUARE_BRACKET;
            token_push(list, &t);
            advance(&pos, string);
            continue;
        }

        if (string[*i] == ']') {
            t.pos = pos;
            t.type = TT_R_SQUARE_BRACKET;
            token_push(list, &t);
            advance(&pos, string);
            continue;
        }

        if (string[*i] == '"') {
            advance(&pos, string); // consume '"'
            parse_string(list, string, &pos, i);
            continue;
        }

        if (isdigit(string[*i])) {
            t.pos = pos;
            t.type = TT_IMMEDIATE;
            i64 num = parse_num(string, &pos, i);
            t.value = malloc(sizeof(i64));
            memcpy(t.value, &num, sizeof(i64));
            token_push(list, &t);
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

            token_push(list, &t);
            continue;
        }

        advance(&pos, string);
    }

    Token t;
    t.value = nullptr;
    t.pos = pos;
    t.type = TT_EOF;
    token_push(list, &t);
}
