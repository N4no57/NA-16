#include "tokens.h"

#include <stdlib.h>
#include <string.h>

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

void token_push(TokenList* list, const Token *token) {
    if (!list) return;

    if (list->count+1 >= list->size) {
        list->size *= 2;
        Token *tmp = realloc(list->tokens, list->size * sizeof(Token));
        if (!tmp) {
            exit(1);
        }
        list->tokens = tmp;
    }

    memcpy(&list->tokens[list->count], token, sizeof(Token));

    list->count++;
}

void token_insert(TokenList* list, const Token *token, const u64 idx) {
    if (idx >= list->size) return;

    if (list->count >= list->size) {
        list->size *= 2;
        Token *tmp = realloc(list->tokens, list->size * sizeof(Token));
        if (!tmp) {
            exit(1);
        }
        list->tokens = tmp;
    }

    const size_t move_count = list->size - idx;
    memmove(&list->tokens[idx + 1], &list->tokens[idx], move_count * sizeof(Token));

    memcpy(&list->tokens[idx], token, sizeof(Token));

    list->count++;
}

void token_delete(TokenList* list, const u64 idx) {
    if (idx >= list->size) return;

    const size_t move_count = list->size - idx - 1;

    if (move_count > 0) {
        memmove(&list->tokens[idx], &list->tokens[idx + 1], move_count * sizeof(Token));
    }

    list->count--;
}
