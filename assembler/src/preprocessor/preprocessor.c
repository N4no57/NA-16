#include "preprocessor.h"
#include "../lexer/lexer.h"

#include <stdlib.h>
#include <string.h>

// expects idx to be set to the .include directive
void handle_include(TokenList *tokens, const u64 *idx, Token *tok) {
    token_delete(tokens, *idx); // remove .include directive
    char *filename = strdup(tok->value);
    token_delete(tokens, *idx); // remove argument after .include

    TokenList new_tokens;
    init_TokenList(&new_tokens);

    char *code = read_assembly(filename);

    tokenise(&new_tokens, (u8 *)filename, (u8 *)code);

    new_tokens.count--; // "pop" EOF token at the end

    for (u64 i = 0; i < new_tokens.count; i++) {
        token_insert(tokens, &new_tokens.tokens[i], i + *idx);
    }

    free(code);
    free(new_tokens.tokens);
}

void include_expander(TokenList *tokens) {
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