#include "parser.h"
#include "../lib/error.h"

#include <stdlib.h>

void init_parser(NodeProgram *ast) {
    ast->count = 0;
    ast->size = 8;
    ast->statements = malloc(sizeof(NodeStatement) * ast->size);
    if (!ast->statements) exit(1);
}

void consume(TokenList *tokens, u64 *idx, Token *tok) {
    (*idx)++;
    *tok = tokens->tokens[*idx];
}

void parse_operand(NodeOperand *operand, TokenList *tokens, u64 *idx, Token *tok) {
    operand->pos = tok->pos;
    if (tok->type == TT_REGISTER) {
        operand->kind = REGISTER;
        operand->reg = getregister(tok->value);
        consume(tokens, idx, tok); // consume reg
    } else if (tok->type == TT_IMMEDIATE) {
        operand->kind = IMMEDIATE;
        operand->immediate = *(i64 *)tok->value;
        consume(tokens, idx, tok); // consume immediate
    } else if (tok->type == TT_PLUS || tok->type == TT_MINUS) {
        // special shit for conditional jumps
        operand->kind = DISPLACEMENT;
        bool sign = tok->type == TT_MINUS;
        consume(tokens, idx, tok); // consume '+'/'-'

        if (tok->type != TT_IMMEDIATE) {
            error(tok->pos, "Expected an immediate after '+'/'-'");
            return;
        }

        operand->immediate = *(i64 *)tok->value;
        if (sign) {
            operand->immediate *= -1;
        }

        consume(tokens, idx, tok); // consume number
    } else if (tok->type == TT_L_SQUARE_BRACKET) {
        // register indirect only (for now)
        operand->kind = REG_INDIRECT;
        consume(tokens, idx, tok); // consume '['

        if (tok->type != TT_REGISTER) {
            error(tok->pos, "Expected a register");
            return;
        }

        operand->reg = getregister(tok->value);
        consume(tokens, idx, tok); // consume reg

        if (tok->type != TT_R_SQUARE_BRACKET) {
            error(tok->pos, "Expected a ']'");
            return;
        }

        consume(tokens, idx, tok); // consume ']'
    } else if (tok->type == TT_IDENTIFIER) {
        operand->kind = SYMBOL;
        operand->symbol_name = tok->value;
        consume(tokens, idx, tok);
    } else {
        error(operand->pos, "Unknown operand type");
        consume(tokens, idx, tok);
    }
}

void parse_instruction(NodeInstruction *instruction, TokenList *tokens, u64 *idx, Token *tok) {
    instruction->mnemonic = tok->value;
    instruction->pos = tok->pos;
    consume(tokens, idx, tok);

    while (tok->type != TT_EOF || tok->type == TT_COMMA) {
        if (tok->type == TT_NEWLINE) {
            consume(tokens, idx, tok);
            break;
        }

        if (tok->type == TT_COMMA) consume(tokens, idx, tok);

        if (tok->type == TT_SIZESPEC) {
            instruction->operand_size = getsizespec(tok->value);
            consume(tokens, idx, tok);
        }

        parse_operand(&instruction->operands[instruction->operand_count++], tokens, idx, tok);
    }
}

void parse_symbol(NodeSymbol *sym, TokenList *tokens, u64 *idx, Token *tok) {
    /*
     * <Symbol> ::= TT_IDENTIFIER ':'
     *            | TT_IDENTIFIER '=' TT_IMMEDIATE
     */

    sym->pos = tok->pos;
    sym->symbol_name = tok->value;
    consume(tokens, idx, tok);

    if (tok->type == TT_COLON) {
        consume(tokens, idx, tok);
        sym->value = -1;
        sym->kind = SK_LABEL;
    } else if (tok->type == TT_EQUALS) {
        consume(tokens, idx, tok);

        if (tok->type != TT_IMMEDIATE) {
            error(tok->pos, "Expected an immediate after '='");
        }

        sym->value = *(i32 *)tok->value;
        sym->kind = SK_CONSTANT;
        consume(tokens, idx, tok);
    } else {
        error(tok->pos, "Expected ':' or '=' followed by an immediate");
        consume(tokens, idx, tok);
    }
}

NodeStatement parse_statement(NodeProgram *ast, TokenList *tokens, u64 *idx, Token *tok) {
    NodeStatement ret_val = {0};

    if (tok->type == TT_MNEMONIC) {
        parse_instruction(&ret_val.instruction, tokens, idx, tok);
        ret_val.kind = ST_INSTRUCTION;
    } else if (tok->type == TT_IDENTIFIER) {
        parse_symbol(&ret_val.symbol, tokens, idx, tok);
        ret_val.kind = ST_SYMBOL;
    } else if (tok->type == TT_NEWLINE) {
        consume(tokens, idx, tok);
    } else {
        error(tok->pos, "Unknown statement type");
        consume(tokens, idx, tok);
    }

    return ret_val;
}

void parse(NodeProgram *ast, TokenList *tokens) {
    if (!ast || !tokens) return;
    init_parser(ast);

    u64 idx = 0;
    Token *tok = &tokens->tokens[idx];

    while (tok->type != TT_EOF) {
        if (ast->count >= ast->size) {
            ast->size *= 2;
            NodeStatement *tmp = realloc(ast->statements, ast->size * sizeof(NodeStatement));
            if (!tmp) exit(1);
            ast->statements = tmp;
        }
        NodeStatement node = parse_statement(ast, tokens, &idx, tok);
        if (node.kind == ST_NONE) continue;
        ast->statements[ast->count] = node;
        ast->count++;
    }
}
