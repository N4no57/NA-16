#include "preprocessor.h"
#include "../lexer/lexer.h"
#include "../lib/error.h"

#include <stdlib.h>
#include <string.h>

// expects idx to be set to the .include directive
void handle_include(TokenList *list, const u64 *idx, Token *tok) {
    token_delete(list, *idx); // remove .include directive
    if (list->tokens[*idx].type != TT_STRING) {
        error(list->tokens[*idx].pos, "Expected a string after \".include\"");
        return;
    }
    String *s = tok->value; // the actual string has no null terminator so we add it in manually

    char *filename = malloc(sizeof(char) * s->size + 1);
    memcpy(filename, s->str, s->size);
    filename[s->size] = '\0';

    token_delete(list, *idx); // remove argument after .include

    TokenList new_tokens;
    init_TokenList(&new_tokens);

    char *code = read_assembly(filename);

    tokenise(&new_tokens, (u8 *)filename, (u8 *)code);

    new_tokens.count--; // "pop" EOF token at the end

    for (u64 i = 0; i < new_tokens.count; i++) {
        token_insert(list, &new_tokens.tokens[i], i + *idx);
    }

    free(code);
    free(new_tokens.tokens);
}

void include_expander(TokenList *tokens) { // TODO: handle circular dependency
    u64 idx = 0;
    Token *curr = &tokens->tokens[idx];
    while (curr->type != TT_EOF) {
        if (curr->type == TT_DIRECTIVE) {
            if (strcmp(curr->value, ".include") == 0) {
                handle_include(tokens, &idx, curr);
            }
        }

        curr = &tokens->tokens[++idx];
    }
}

void preprocessor(TokenList *tokens) {
    include_expander(tokens);
}