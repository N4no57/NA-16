#include "parser.h"
#include "../lib/error.h"

#include <stdlib.h>

void init_parser(NodeProgram *ast) {
    ast->count = 0;
    ast->size = 8;
    ast->statements = malloc(sizeof(NodeStatement) * ast->size);
    if (!ast->statements) exit(1);
}

void consume_tok(const TokenList *tokens, u64 *idx, Token *tok) {
    (*idx)++;
    *tok = tokens->tokens[*idx];
}

void parse_memory_op(NodeOperand *operand, const TokenList *tokens, u64 *idx, Token *tok) {
    // stuff like [10], [r0], [r0+1]\[r0-1], [r0+r1*1] and [r0+r1*1+10]
    consume_tok(tokens, idx, tok); // consume '['

    if (tok->type == TT_IMMEDIATE) {
        operand->kind = ABSOLUTE;
        operand->immediate = *(i64 *)tok->value;
        consume_tok(tokens, idx, tok); // consume immediate

        if (tok->type != TT_R_SQUARE_BRACKET) {
            error(tok->pos, "Expected a ']'");
            return;
        }

        consume_tok(tokens, idx, tok); // consume ']'
        return;
    }

    if (tok->type == TT_REGISTER) {
        operand->reg = *(i64 *)tok->value;
        consume_tok(tokens, idx, tok);
        if (tok->type == TT_PLUS || tok->type == TT_MINUS) {
            // either register indirect with displacement, SIB or SIB with displacement
            bool sign = tok->type == TT_MINUS;
            consume_tok(tokens, idx, tok); // consume '+'/'-'
            if (tok->type == TT_IMMEDIATE) {
                // reg indirect with displacement
                operand->kind = REG_IND_DISP;

                operand->immediate = *(i64 *)tok->value;
                if (sign) {
                    operand->immediate *= -1;
                }

                consume_tok(tokens, idx, tok); // consume number
            } else if (tok->type == TT_REGISTER) {
                // SIB or SIB with displacement
            }
        } else {
            operand->kind = REG_INDIRECT;
            consume_tok(tokens, idx, tok); // consume reg
        }

        if (tok->type != TT_R_SQUARE_BRACKET) {
            error(tok->pos, "Expected a ']'");
            return;
        }

        consume_tok(tokens, idx, tok); // consume ']'
    } else {
        error(tok->pos, "Expected a register or an immediate inside the square brackets");
        return;
    }

    operand->reg = getregister(tok->value);
    consume_tok(tokens, idx, tok); // consume reg

    if (tok->type != TT_R_SQUARE_BRACKET) {
        error(tok->pos, "Expected a ']'");
        return;
    }

    consume_tok(tokens, idx, tok); // consume ']'
}

void parse_operand(NodeOperand *operand, TokenList *tokens, u64 *idx, Token *tok) {
    operand->pos = tok->pos;
    if (tok->type == TT_REGISTER) {
        operand->kind = REGISTER;
        operand->reg = getregister(tok->value);
        consume_tok(tokens, idx, tok); // consume reg
    } else if (tok->type == TT_IMMEDIATE) {
        operand->kind = IMMEDIATE;
        operand->immediate = *(i64 *)tok->value;
        consume_tok(tokens, idx, tok); // consume immediate
    } else if (tok->type == TT_PLUS || tok->type == TT_MINUS) {
        // special shit for conditional jumps
        operand->kind = DISPLACEMENT;
        bool sign = tok->type == TT_MINUS;
        consume_tok(tokens, idx, tok); // consume '+'/'-'

        if (tok->type != TT_IMMEDIATE) {
            error(tok->pos, "Expected an immediate after '+'/'-'");
            return;
        }

        operand->immediate = *(i64 *)tok->value;
        if (sign) {
            operand->immediate *= -1;
        }

        consume_tok(tokens, idx, tok); // consume number
    } else if (tok->type == TT_L_SQUARE_BRACKET) {
        parse_memory_op(operand, tokens, idx, tok);
    } else if (tok->type == TT_IDENTIFIER) {
        operand->kind = SYMBOL;
        operand->symbol_name = tok->value;
        consume_tok(tokens, idx, tok);
    } else {
        error(operand->pos, "Unknown operand type");
        consume_tok(tokens, idx, tok);
    }
}

void parse_instruction(NodeInstruction *instruction, TokenList *tokens, u64 *idx, Token *tok) {
    instruction->mnemonic = tok->value;
    instruction->pos = tok->pos;
    consume_tok(tokens, idx, tok);

    while (tok->type != TT_EOF || tok->type == TT_COMMA) {
        if (tok->type == TT_NEWLINE) {
            consume_tok(tokens, idx, tok);
            break;
        }

        if (tok->type == TT_COMMA) consume_tok(tokens, idx, tok);

        if (tok->type == TT_SIZESPEC) {
            instruction->operand_size = getsizespec(tok->value);
            consume_tok(tokens, idx, tok);
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
    consume_tok(tokens, idx, tok);

    if (tok->type == TT_COLON) {
        consume_tok(tokens, idx, tok);
        sym->value = -1;
        sym->kind = SK_LABEL;
    } else if (tok->type == TT_EQUALS) {
        consume_tok(tokens, idx, tok);

        if (tok->type != TT_IMMEDIATE) {
            error(tok->pos, "Expected an immediate after '='");
        }

        sym->value = *(i32 *)tok->value;
        sym->kind = SK_CONSTANT;
        consume_tok(tokens, idx, tok);
    } else {
        error(tok->pos, "Expected ':' or '=' followed by an immediate");
        consume_tok(tokens, idx, tok);
    }
}

void parse_directive(NodeDirective *directive, TokenList *tokens, u64 *idx, Token *tok) {
    directive->pos = tok->pos;
    directive->name = tok->value;
    consume_tok(tokens, idx, tok);

    init_TokenList(&directive->args);

    while (tok->type != TT_EOF) {
        if (tok->type == TT_NEWLINE) {
            consume_tok(tokens, idx, tok);
            return;
        }

        if (tok->type == TT_COMMA) {
            consume_tok(tokens, idx, tok);
            continue;
        }

        token_push(&directive->args, tok);
        consume_tok(tokens, idx, tok);
    }
}

NodeStatement parse_statement(TokenList *tokens, u64 *idx, Token *tok) {
    NodeStatement ret_val = {0};

    if (tok->type == TT_MNEMONIC) {
        parse_instruction(&ret_val.instruction, tokens, idx, tok);
        ret_val.kind = ST_INSTRUCTION;
    } else if (tok->type == TT_IDENTIFIER) {
        parse_symbol(&ret_val.symbol, tokens, idx, tok);
        ret_val.kind = ST_SYMBOL;
    } else if (tok->type == TT_DIRECTIVE) {
        parse_directive(&ret_val.directive, tokens, idx, tok);
        ret_val.kind = ST_DIRECTIVE;
    } else if (tok->type == TT_NEWLINE) {
        consume_tok(tokens, idx, tok);
    } else {
        error(tok->pos, "Unknown statement type");
        consume_tok(tokens, idx, tok);
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
        NodeStatement node = parse_statement(tokens, &idx, tok);
        if (node.kind == ST_NONE) continue;
        ast->statements[ast->count] = node;
        ast->count++;
    }
}
