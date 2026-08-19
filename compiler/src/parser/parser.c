#include "parser.h"
#include "../type.h"

#include <stdlib.h>
#include <string.h>

Error parse_declaration_specifiers(Parser *p, DeclarationSpecifiers *result);
Error parse_declarator(Parser *p, Declarator **result);
Error parse_declaration(Parser *p, Declaration *declaration);

void parser_init(Parser *p, TokenStream *tokens) {
    p->tokens = tokens;

    vector_init(&p->errors, sizeof(ParserError));
}

static const CToken *parser_peek(const Parser *p, size_t lookahead) {
    return token_stream_peek(p->tokens, lookahead);
}

static Error parser_consume(const Parser *p, CToken *token) {
    return token_stream_consume(p->tokens, token);
}

static Error parser_match(const Parser *p, const CTokenKind expected) {
    return token_stream_match(p->tokens, expected);
}

static Error parser_expected(Parser *p, const CTokenKind expected, CToken *result) {
    const CToken *token = parser_peek(p, 0);

    if (token == nullptr) {
        return ERROR_NULL_POINTER;
    }

    if (token->kind != expected) {
        // p->error.span = token->span;
        // p->error.message = "Unexpected token";

        return ERROR_NOT_FOUND;
    }

    if (result != nullptr) {
        *result = *token;
    }

    return token_stream_consume(p->tokens, nullptr);
}

static bool is_storage_class_specifier(const CTokenKind tok) {
    switch (tok) {
        case C_TOKEN_KW_TYPEDEF:
        case C_TOKEN_KW_EXTERN:
        case C_TOKEN_KW_STATIC:
        case C_TOKEN_KW_AUTO:
        case C_TOKEN_KW_REGISTER:
            return true;
        default:
            return false;
    }
}

static bool is_type_specifier(const CTokenKind tok) {
    switch (tok) {
        case C_TOKEN_KW_VOID:
        case C_TOKEN_KW_CHAR:
        case C_TOKEN_KW_SHORT:
        case C_TOKEN_KW_INT:
        case C_TOKEN_KW_LONG:
        case C_TOKEN_KW_FLOAT:
        case C_TOKEN_KW_DOUBLE:
        case C_TOKEN_KW_SIGNED:
        case C_TOKEN_KW_UNSIGNED:
        case C_TOKEN_KW__BOOL:
        case C_TOKEN_KW__COMPLEX:
        // TODO structs, enums and typedef names
            return true;
        default:
            return false;
    }
}

static bool is_type_qualifier(const CTokenKind tok) {
    switch (tok) {
        case C_TOKEN_KW_CONST:
        case C_TOKEN_KW_RESTRICT:
        case C_TOKEN_KW_VOLATILE:
            return true;
        default:
            return false;
    }
}

static bool is_function_specifier(const CTokenKind tok) {
    switch (tok) {
        case C_TOKEN_KW_INLINE:
            return true;
        default:
            return false;
    }
}

static bool is_declaration_specifier(const CTokenKind tok) {
    return is_storage_class_specifier(tok) || is_type_specifier(tok) || is_type_qualifier(tok);
}

static bool is_expression_start(const CTokenKind tok) {
    switch (tok) {
        case C_TOKEN_LEFT_PAREN:
        case C_TOKEN_IDENTIFIER:
        case C_TOKEN_CHARACTER_CONSTANT:
        case C_TOKEN_INTEGER_CONSTANT:
        case C_TOKEN_FLOATING_CONSTANT:
        case C_TOKEN_STRING_LITERAL:
        case C_TOKEN_INCREMENT:
        case C_TOKEN_DECREMENT:
        case C_TOKEN_ASTERISK:
        case C_TOKEN_AMPERSAND:
        case C_TOKEN_MINUS:
        case C_TOKEN_PLUS:
        case C_TOKEN_EXCLAMATION:
        case C_TOKEN_TILDE:
        case C_TOKEN_KW_SIZEOF:
            return true;
        default:
            return false;
    }
}

constexpr BinaryOp TT_to_BinaryOp[C_TOKEN_COUNT] = {
    [C_TOKEN_ASTERISK] = BIN_OP_MULTIPLY,
    [C_TOKEN_SLASH] = BIN_OP_DIVIDE,
    [C_TOKEN_PERCENT] = BIN_OP_MOD,
    [C_TOKEN_PLUS] = BIN_OP_ADD,
    [C_TOKEN_MINUS] = BIN_OP_SUBTRACT,
    [C_TOKEN_SHIFT_LEFT] = BIN_OP_SHIFT_LEFT,
    [C_TOKEN_SHIFT_RIGHT] = BIN_OP_SHIFT_RIGHT,
    [C_TOKEN_LESS] = BIN_OP_LESS,
    [C_TOKEN_GREATER] = BIN_OP_GREATER,
    [C_TOKEN_LESS_EQUAL] = BIN_OP_LESS_OR_EQUAL,
    [C_TOKEN_GREATER_EQUAL] = BIN_OP_GREATER_OR_EQUAL,
    [C_TOKEN_ASSIGN] = BIN_OP_EQUAL,
    [C_TOKEN_ADD_ASSIGN] = BIN_OP_EQUAL,
    [C_TOKEN_SUBTRACT_ASSIGN] = BIN_OP_EQUAL,
    [C_TOKEN_MULTIPLY_ASSIGN] = BIN_OP_EQUAL,
    [C_TOKEN_DIVIDE_ASSIGN] = BIN_OP_EQUAL,
    [C_TOKEN_REMAINDER_ASSIGN] = BIN_OP_EQUAL,
    [C_TOKEN_SHIFT_LEFT_ASSIGN] = BIN_OP_EQUAL,
    [C_TOKEN_SHIFT_RIGHT_ASSIGN] = BIN_OP_EQUAL,
    [C_TOKEN_AND_ASSIGN] = BIN_OP_EQUAL,
    [C_TOKEN_XOR_ASSIGN] = BIN_OP_EQUAL,
    [C_TOKEN_OR_ASSIGN] = BIN_OP_EQUAL,
    [C_TOKEN_NOT_EQUAL] = BIN_OP_NOT_EQUAL,
    [C_TOKEN_AMPERSAND] = BIN_OP_BITWISE_AND,
    [C_TOKEN_CARET] = BIN_OP_EXCLUSIVE_OR,
    [C_TOKEN_PIPE] = BIN_OP_INCLUSIVE_OR,
    [C_TOKEN_LOGICAL_AND] = BIN_OP_LOGICAL_AND,
    [C_TOKEN_LOGICAL_OR] = BIN_OP_LOGICAL_OR,
    [C_TOKEN_COMMA] = BIN_OP_COMMA
};

enum {
    BP_COMMA          = 10,
    BP_ASSIGNMENT     = 20,
    BP_CONDITIONAL    = 30,
    BP_LOGICAL_OR     = 40,
    BP_LOGICAL_AND    = 50,
    BP_BITWISE_OR     = 60,
    BP_BITWISE_XOR    = 70,
    BP_BITWISE_AND    = 80,
    BP_EQUALITY       = 90,
    BP_RELATIONAL     = 100,
    BP_SHIFT          = 110,
    BP_ADDITIVE       = 120,
    BP_MULTIPLICATIVE = 130,
    BP_CAST           = 140,
    BP_PREFIX         = 150,
    BP_POSTFIX        = 160,
};

typedef struct {
    int left_bp;
    int right_bp;
    bool valid;
} BinaryOperatorInfo;

static BinaryOperatorInfo get_infix_info(const CTokenKind operator) {
    BinaryOperatorInfo info = {0};
    switch (operator) {
        case C_TOKEN_COMMA:
            info.left_bp = info.right_bp = BP_COMMA;
            info.right_bp++;
            break;
        case C_TOKEN_ASSIGN:
            info.left_bp = info.right_bp = BP_ASSIGNMENT;
            break;
        case C_TOKEN_QUESTION:
            break; // TODO conditional
        case C_TOKEN_LOGICAL_OR:
            info.left_bp = info.right_bp = BP_LOGICAL_OR;
            info.right_bp++;
            break;
        case C_TOKEN_LOGICAL_AND:
            info.left_bp = info.right_bp = BP_LOGICAL_AND;
            info.right_bp++;
            break;
        case C_TOKEN_PIPE:
            info.left_bp = info.right_bp = BP_BITWISE_OR;
            info.right_bp++;
            break;
        case C_TOKEN_CARET:
            info.left_bp = info.right_bp = BP_BITWISE_XOR;
            info.right_bp++;
            break;
        case C_TOKEN_AMPERSAND:
            info.left_bp = info.right_bp = BP_BITWISE_AND;
            info.right_bp++;
            break;
        case C_TOKEN_EQUAL_EQUAL:
        case C_TOKEN_NOT_EQUAL:
            info.left_bp = info.right_bp = BP_EQUALITY;
            info.right_bp++;
            break;
        case C_TOKEN_LESS:
        case C_TOKEN_LESS_EQUAL:
        case C_TOKEN_GREATER:
        case C_TOKEN_GREATER_EQUAL:
            info.left_bp = info.right_bp = BP_RELATIONAL;
            info.right_bp++;
            break;
        case C_TOKEN_SHIFT_LEFT:
        case C_TOKEN_SHIFT_RIGHT:
            info.left_bp = info.right_bp = BP_SHIFT;
            info.right_bp++;
            break;
        case C_TOKEN_MINUS:
        case C_TOKEN_PLUS:
            info.left_bp = info.right_bp = BP_ADDITIVE;
            info.right_bp++;
            break;
        case C_TOKEN_ASTERISK:
        case C_TOKEN_SLASH:
        case C_TOKEN_PERCENT:
            info.left_bp = info.right_bp = BP_MULTIPLICATIVE;
            info.right_bp++;
            break;
        default:
            return info;
    }

    info.valid = true;

    return info;
}

Expr *parse_expression(Parser *p, int min_bp);

Expr *parse_prefix(Parser *p) {
    Expr *ret_val = malloc(sizeof(Expr));
    CToken tok = *parser_peek(p, 0);

    ret_val->kind = EXPR_UNARY;

    switch (tok.kind) {
        case C_TOKEN_IDENTIFIER:
            if (parser_consume(p, nullptr) != ERROR_OK) return nullptr; // TODO error
            ret_val->kind = EXPR_IDENTIFIER;
            ret_val->identifier.name = tok;
            return ret_val;

        case C_TOKEN_INTEGER_CONSTANT:
        case C_TOKEN_FLOATING_CONSTANT:
        case C_TOKEN_CHARACTER_CONSTANT:
            if (parser_consume(p, nullptr) != ERROR_OK) return nullptr; // TODO error
            ret_val->kind = EXPR_CONSTANT;

            if (tok.kind == C_TOKEN_INTEGER_CONSTANT) {
                ret_val->constant.kind = CONSTANT_INTEGER;
                ret_val->constant.integer.suffix = tok.data.integer.suffix;
                ret_val->constant.integer.unsigned_int = tok.data.integer.unsigned_int;
            } else if (tok.kind == C_TOKEN_FLOATING_CONSTANT) {
                ret_val->constant.kind = CONSTANT_FLOAT;
                ret_val->constant.floating = tok.data.floating_point.floating;
            } else {
                ret_val->constant.kind = CONSTANT_CHAR;
                ret_val->constant.character_or_str = tok.data.string_or_character.str;
            }

            return ret_val;

        case C_TOKEN_STRING_LITERAL:
            if (parser_consume(p, nullptr) != ERROR_OK) return nullptr; // TODO error
            ret_val->kind = EXPR_CONSTANT;

            ret_val->constant.kind = CONSTANT_STRING_LITERAL;
            ret_val->constant.character_or_str = tok.data.string_or_character.str;

            return ret_val;

        case C_TOKEN_INCREMENT:
            if (parser_consume(p, nullptr) != ERROR_OK) return nullptr; // TODO error
            ret_val->unary.op = OP_PREFIX_INCREMENT;
            ret_val->unary.operand = parse_expression(p, BP_PREFIX);
            return ret_val;

        case C_TOKEN_DECREMENT:
            if (parser_consume(p, nullptr) != ERROR_OK) return nullptr; // TODO error
            ret_val->unary.op = OP_PREFIX_DECREMENT;
            ret_val->unary.operand = parse_expression(p, BP_PREFIX);
            return ret_val;

        case C_TOKEN_AMPERSAND:
            if (parser_consume(p, nullptr) != ERROR_OK) return nullptr; // TODO error
            ret_val->unary.op = OP_AMPERSAND;
            ret_val->unary.operand = parse_expression(p, BP_PREFIX);
            return ret_val;

        case C_TOKEN_ASTERISK:
            if (parser_consume(p, nullptr) != ERROR_OK) return nullptr; // TODO error
            ret_val->unary.op = OP_ASTERISK;
            ret_val->unary.operand = parse_expression(p, BP_PREFIX);
            return ret_val;

        case C_TOKEN_PLUS:
            if (parser_consume(p, nullptr) != ERROR_OK) return nullptr; // TODO error
            ret_val->unary.op = OP_PLUS;
            ret_val->unary.operand = parse_expression(p, BP_PREFIX);
            return ret_val;

        case C_TOKEN_MINUS:
            if (parser_consume(p, nullptr) != ERROR_OK) return nullptr; // TODO error
            ret_val->unary.op = OP_MINUS;
            ret_val->unary.operand = parse_expression(p, BP_PREFIX);
            return ret_val;

        case C_TOKEN_TILDE:
            if (parser_consume(p, nullptr) != ERROR_OK) return nullptr; // TODO error
            ret_val->unary.op = OP_BITWISE_NOT;
            ret_val->unary.operand = parse_expression(p, BP_PREFIX);
            return ret_val;

        case C_TOKEN_EXCLAMATION:
            if (parser_consume(p, nullptr) != ERROR_OK) return nullptr; // TODO error
            ret_val->unary.op = OP_LOGICAL_NOT;
            ret_val->unary.operand = parse_expression(p, BP_PREFIX);
            return ret_val;

        case C_TOKEN_KW_SIZEOF:
            if (parser_consume(p, nullptr) != ERROR_OK) return nullptr; // TODO error
            return ret_val; // TODO

        case C_TOKEN_LEFT_PAREN:
            if (parser_consume(p, nullptr) != ERROR_OK) return nullptr; // TODO error

            tok = *parser_peek(p, 0);

            if (tok.kind == C_TOKEN_IDENTIFIER &&
                parser_peek(p, 1)->kind == C_TOKEN_RIGHT_PAREN) {
                // assume this is a cast and semantic analyzer will fix ts
                /* There is this but this is a TODO
                 * ( type-name ){ initializer-list }
                 * ( type-name ){ initializer-list ,}
                 */

                ret_val->kind = EXPR_CAST;
                ret_val->unary.op = OP_CAST;
                ret_val->unary.operand = parse_expression(p, BP_CAST);
            } else {
                ret_val = parse_expression(p, 0);
            }

            if (parser_expected(p, C_TOKEN_RIGHT_PAREN, nullptr) != ERROR_OK) return nullptr; // TODO error

            return ret_val;

        default:
            break;
    }

    free(ret_val);
    return nullptr; // TODO error
}

Expr *parse_infix(Parser *p, Expr *lhs, const CToken tok, BinaryOperatorInfo op) {
    Expr *binary_op = malloc(sizeof(Expr));
    if (binary_op == nullptr) return nullptr; // TODO error

    binary_op->binary.op = TT_to_BinaryOp[tok.kind];
    binary_op->binary.op_tok = tok;

    if (binary_op->binary.op == BIN_OP_EQUAL) binary_op->kind = EXPR_ASSIGN;
    else binary_op->kind = EXPR_BINARY;

    binary_op->binary.left = lhs;
    binary_op->binary.right = parse_expression(p, op.right_bp);

    return binary_op;
}

Error parse_statement(Parser *p, Statement *result);

Expr *parse_expression(Parser *p, int min_bp) {
    Expr *lhs = parse_prefix(p);

    while (true) {
        const CToken tok = *parser_peek(p, 0);

        if (tok.kind == C_TOKEN_EOF) break;

        // postfix
        if (tok.kind == C_TOKEN_LEFT_BRACKET) {
            if (BP_POSTFIX < min_bp) break;

            if (parser_consume(p, nullptr) != ERROR_OK) return nullptr; // TODO error

            Expr *subscript = malloc(sizeof(Expr));
            if (subscript == nullptr) return nullptr; // TODO error

            subscript->kind = EXPR_SUBSCRIPT;

            subscript->binary.left = lhs;
            subscript->binary.right = parse_expression(p, 0);

            if (parser_expected(p, C_TOKEN_RIGHT_BRACKET, nullptr)) {
                expression_destroy(subscript);
                return nullptr; // TODO error
            }

            lhs = subscript;
            continue;
        }

        if (tok.kind == C_TOKEN_LEFT_PAREN) {
            if (BP_POSTFIX < min_bp) break;

            if (parser_consume(p, nullptr) != ERROR_OK) {
                expression_destroy(lhs);
                return nullptr; // TODO error
            }

            Vector arg_list;
            if (vector_init(&arg_list, sizeof(Expr *))) {
                expression_destroy(lhs);
                return nullptr; // TODO error
            }


            while (parser_peek(p, 0)->kind != C_TOKEN_RIGHT_PAREN) { // TODO free every expression in the arg list
                if (parser_peek(p, 0)->kind == C_TOKEN_EOF) return nullptr; // TODO error

                Expr *arg = parse_expression(p, BP_ASSIGNMENT);
                if (arg == nullptr) {
                    expression_destroy(lhs);
                    return nullptr; // TODO error
                }

                if (parser_expected(p, C_TOKEN_COMMA, nullptr)) {
                    vector_destroy(&arg_list);
                    expression_destroy(lhs);
                    return nullptr; // TODO error
                }

                if (parser_peek(p, 0)->kind == C_TOKEN_RIGHT_PAREN) break;
                if (vector_push(&arg_list, &arg) != ERROR_OK) {
                    expression_destroy(lhs);
                    expression_destroy(arg);
                    return nullptr; // TODO error
                }
            }

            if (parser_expected(p, C_TOKEN_RIGHT_PAREN, nullptr)) {
                expression_destroy(lhs);
                return nullptr; // TODO error
            }

            Expr *func_call = malloc(sizeof(Expr));
            func_call->kind = EXPR_CALL;
            func_call->function_call.argument_count = arg_list.length;
            func_call->function_call.args = arg_list.data;
            func_call->function_call.name = lhs;

            lhs = func_call;
            continue;
        }

        if (tok.kind == C_TOKEN_DOT ||
            tok.kind == C_TOKEN_ARROW) {
            if (BP_POSTFIX < min_bp) break;

            if (parser_consume(p, nullptr) != ERROR_OK) return nullptr; // TODO error

            Expr *member = malloc(sizeof(Expr));
            if (member == nullptr) return nullptr; // TODO error

            member->kind = EXPR_MEMBER;
            member->member.dereference = tok.kind == C_TOKEN_ARROW;
            member->member.member = lhs;

            if (parser_expected(p, C_TOKEN_IDENTIFIER, &member->member.member_item) != ERROR_OK) {
                return nullptr; // TODO error
            }

            lhs = member;
            continue;
        }

        if (tok.kind == C_TOKEN_INCREMENT ||
            tok.kind == C_TOKEN_DECREMENT) {
            if (BP_POSTFIX < min_bp) break;

            if (parser_consume(p, nullptr) != ERROR_OK) return nullptr; // TODO error

            Expr *inc_dec = malloc(sizeof(Expr));
            if (inc_dec == nullptr) return nullptr; // TODO error

            inc_dec->kind = EXPR_UNARY;
            inc_dec->unary.op = tok.kind == C_TOKEN_INCREMENT ? OP_POSTFIX_INCREMENT : OP_POSTFIX_DECREMENT;
            inc_dec->unary.operand = lhs;

            lhs = inc_dec;
            continue;
        }

        if (tok.kind == C_TOKEN_QUESTION) {
            if (BP_CONDITIONAL < min_bp) break;

            if (parser_consume(p, nullptr) != ERROR_OK) return nullptr; // TODO error

            Expr *conditional = malloc(sizeof(Expr));
            if (conditional == nullptr) return nullptr; // TODO error

            conditional->kind = EXPR_CONDITIONAL;
            conditional->conditional.expression = lhs;
            conditional->conditional.true_expression = parse_expression(p, 0);

            if (parser_expected(p, C_TOKEN_COLON, nullptr)) {
                expression_destroy(conditional);
                return nullptr; // TODO error
            }

            conditional->conditional.false_expression = parse_expression(p, BP_CONDITIONAL);

            lhs = conditional;
            continue;
        }

        // normal goo goo gaa gaa shit
        const BinaryOperatorInfo op = get_infix_info(tok.kind);

        if (!op.valid) break;
        if (op.left_bp < min_bp) break;

        parser_consume(p, nullptr);

        lhs = parse_infix(p, lhs, tok, op);
    }

    return lhs;
}

Error parse_labeled_statement(Parser *p, Statement *result) {
    Error code;
    SourceSpan span;

    const CToken *tok = parser_peek(p, 0);
    result->kind = STATEMENT_LABELED;

    span.begin = tok->span.begin;

    if (tok->kind == C_TOKEN_IDENTIFIER) {
        result->data.labeled_statement.kind = LABELED_LABEL;
        result->data.labeled_statement.data.label = tok->data.string_or_character.str;

        code = parser_consume(p, nullptr);
        if (code != ERROR_OK) return code;
        tok = parser_peek(p, 0);
    } else if (tok->kind == C_TOKEN_KW_CASE) {
        result->data.labeled_statement.kind = LABELED_CASE;

        code = parser_consume(p, nullptr);
        if (code != ERROR_OK) return code;
        tok = parser_peek(p, 0);

        result->data.labeled_statement.data.expression = parse_expression(p, BP_CONDITIONAL);
    } else if (tok->kind == C_TOKEN_KW_DEFAULT) {
        result->data.labeled_statement.kind = LABELED_DEFAULT;

        code = parser_consume(p, nullptr);
        if (code != ERROR_OK) return code;
        tok = parser_peek(p, 0);
    } else {
        return ERROR_INTERNAL; // TODO error
    }

    code = parser_expected(p, C_TOKEN_COLON, nullptr);
    if (code != ERROR_OK) return code;

    span.end = tok->span.end;

    result->span = span;

    result->data.labeled_statement.statement = malloc(sizeof(Statement));
    if (result->data.labeled_statement.statement == nullptr) return ERROR_ALLOCATION_FAILED;
    return parse_statement(p, result->data.labeled_statement.statement);
}

Error parse_selection_statement(Parser *p, Statement *result) {
    Error code;

    const CToken *tok = parser_peek(p, 0);

    result->kind = STATEMENT_SELECTION;

    SelectionStatement *selec_stmt = &result->data.selection_statement;

    if (tok->kind == C_TOKEN_KW_IF) {
        selec_stmt->kind = SELECTION_IF;

        code = parser_consume(p, nullptr);
        if (code != ERROR_OK) return code;

        code = parser_expected(p, C_TOKEN_LEFT_PAREN, nullptr);
        if (code != ERROR_OK) return code;

        selec_stmt->if_stmt.conditional = parse_expression(p, 0);

        code = parser_expected(p, C_TOKEN_RIGHT_PAREN, nullptr);
        if (code != ERROR_OK) return code;

        Statement stmt = {0};

        code = parse_statement(p, &stmt);
        if (code != ERROR_OK) return code;
        tok = parser_peek(p, 0);

        selec_stmt->if_stmt.then_branch = malloc(sizeof(Statement));
        if (selec_stmt->if_stmt.then_branch == nullptr) return ERROR_ALLOCATION_FAILED;
        *selec_stmt->if_stmt.then_branch = stmt;

        if (tok->kind == C_TOKEN_KW_ELSE) {
            code = parser_consume(p, nullptr);
            if (code != ERROR_OK) return code;

            code = parse_statement(p, &stmt);
            if (code != ERROR_OK) return code;

            selec_stmt->if_stmt.else_branch = malloc(sizeof(Statement));
            if (selec_stmt->if_stmt.else_branch == nullptr) return ERROR_ALLOCATION_FAILED;
            *selec_stmt->if_stmt.else_branch = stmt;
        } else {
            selec_stmt->if_stmt.else_branch = nullptr;
        }

        return ERROR_OK;
    }

    if (tok->kind == C_TOKEN_KW_SWITCH) {
        selec_stmt->kind = SELECTION_SWITCH;

        code = parser_consume(p, nullptr);
        if (code != ERROR_OK) return code;

        code = parser_expected(p, C_TOKEN_LEFT_PAREN, nullptr);
        if (code != ERROR_OK) return code;

        selec_stmt->switch_stmt.expression = parse_expression(p, 0);

        code = parser_expected(p, C_TOKEN_RIGHT_PAREN, nullptr);
        if (code != ERROR_OK) return code;

        Statement stmt = {0};

        code = parse_statement(p, &stmt);
        if (code != ERROR_OK) return code;

        selec_stmt->switch_stmt.body = malloc(sizeof(Statement));
        if (selec_stmt->switch_stmt.body == nullptr) return ERROR_ALLOCATION_FAILED;
        *selec_stmt->switch_stmt.body = stmt;

        return ERROR_OK;
    }

    return ERROR_INTERNAL; // TODO error
}

Error parse_while(Parser *p, IterationStatement *result) {
    const CToken *tok = parser_peek(p, 0);

    Error code = parser_expected(p, C_TOKEN_KW_WHILE, nullptr);
    if (code != ERROR_OK) return code;

    code = parser_expected(p, C_TOKEN_LEFT_PAREN, nullptr);
    if (code != ERROR_OK) return code;

    result->while_loop.condition = parse_expression(p, 0);
    if (result->while_loop.condition == nullptr) return ERROR_INTERNAL;

    code = parser_expected(p, C_TOKEN_RIGHT_PAREN, nullptr);
    if (code != ERROR_OK) return code;

    return ERROR_OK;
}

Error parse_iteration_statement(Parser *p, Statement *result) {
    Error code;

    const CToken *tok = parser_peek(p, 0);

    result->kind = STATEMENT_ITERATION;

    IterationStatement *iter_stmt = &result->data.iteration_statement;

    iter_stmt->kind = ITERATION_WHILE;
    if (tok->kind == C_TOKEN_KW_DO) {
        iter_stmt->while_loop.do_while = true;

        code = parser_consume(p, nullptr);
        if (code != ERROR_OK) return code;

        iter_stmt->while_loop.body = malloc(sizeof(Statement));
        if (iter_stmt->while_loop.body == nullptr) return ERROR_ALLOCATION_FAILED;

        code = parse_statement(p, iter_stmt->while_loop.body);
        if (code != ERROR_OK) return code;

        code = parse_while(p, iter_stmt);
        if (code != ERROR_OK) return code;

        code = parser_expected(p, C_TOKEN_SEMICOLON, nullptr);
        if (code != ERROR_OK) return code;

        return ERROR_OK;
    }

    if (tok->kind == C_TOKEN_KW_WHILE) {
        iter_stmt->while_loop.do_while = false;

        code = parse_while(p, iter_stmt);
        if (code != ERROR_OK) return code;

        iter_stmt->while_loop.body = malloc(sizeof(Statement));
        if (iter_stmt->while_loop.body == nullptr) return ERROR_ALLOCATION_FAILED;

        code = parse_statement(p, iter_stmt->while_loop.body);
        if (code != ERROR_OK) return code;

        return ERROR_OK;
    }

    if (tok->kind == C_TOKEN_KW_FOR) {
        iter_stmt->kind = ITERATION_FOR;

        code = parser_consume(p, nullptr);
        if (code != ERROR_OK) return code;

        code = parser_expected(p, C_TOKEN_LEFT_PAREN, nullptr);
        if (code != ERROR_OK) return code;

        if (is_expression_start(parser_peek(p, 0)->kind)) {
            iter_stmt->for_loop.expr1.expr = parse_expression(p, 0);

            code = parser_expected(p, C_TOKEN_SEMICOLON, nullptr);
            if (code != ERROR_OK) return code;
        } else if (is_declaration_specifier(parser_peek(p, 0)->kind)) {
            code = parse_declaration_specifiers(p, &iter_stmt->for_loop.expr1.declaration->specifiers);
            if (code != ERROR_OK) return code;

            code = parse_declaration(p, iter_stmt->for_loop.expr1.declaration);
            if (code != ERROR_OK) return code;
        }

        if (is_expression_start(parser_peek(p, 0)->kind)) {
            iter_stmt->for_loop.expr2 = parse_expression(p, 0);
        }

        code = parser_expected(p, C_TOKEN_SEMICOLON, nullptr);
        if (code != ERROR_OK) return code;

        if (is_expression_start(parser_peek(p, 0)->kind)) {
            iter_stmt->for_loop.expr3 = parse_expression(p, 0);
        }

        code = parser_expected(p, C_TOKEN_RIGHT_PAREN, nullptr);
        if (code != ERROR_OK) return code;

        iter_stmt->for_loop.body = malloc(sizeof(Statement));
        if (iter_stmt->for_loop.body == nullptr) return ERROR_ALLOCATION_FAILED;

        code = parse_statement(p, iter_stmt->for_loop.body);
        if (code != ERROR_OK) return code;

        return ERROR_OK;
    }

    return ERROR_INTERNAL; // TODO error
}

Error parse_jump_statement(Parser *p, Statement *result) {
    Error code;
    CToken semicolon;

    const CToken *tok = parser_peek(p, 0);
    result->kind = STATEMENT_JUMP;

    const CToken keyword_token = *tok;

    if (tok->kind == C_TOKEN_KW_RETURN) {
        result->data.jump_statement.kind = JUMP_STATEMENT_RETURN;
        code = parser_consume(p, nullptr);
        if (code != ERROR_OK) return code;

        Expr *expression = nullptr;

        if (parser_peek(p, 0)->kind != C_TOKEN_SEMICOLON) {
            expression = parse_expression(p, 0);

            if (expression == nullptr) {
                return ERROR_NULL_POINTER;
            }
        }

        if (parser_peek(p, 0)->kind != C_TOKEN_SEMICOLON) expression_destroy(expression);

        result->data.jump_statement.data.return_statement.expression = expression;
    } else if (tok->kind == C_TOKEN_KW_GOTO) {
        result->data.jump_statement.kind = JUMP_STATEMENT_GOTO;

        code = parser_consume(p, nullptr);
        if (code != ERROR_OK) return code;

        CToken identifier;

        code = parser_expected(p, C_TOKEN_IDENTIFIER, &identifier);
        if (code != ERROR_OK) return code;

        result->data.jump_statement.data.goto_statement.label = identifier.data.string_or_character.str;
    } else if (tok->kind == C_TOKEN_KW_BREAK) {
        code = parser_consume(p, nullptr);
        if (code != ERROR_OK) return code;

        result->data.jump_statement.kind = JUMP_STATEMENT_BREAK;
    } else if (tok->kind == C_TOKEN_KW_CONTINUE) {
        code = parser_consume(p, nullptr);
        if (code != ERROR_OK) return code;

        result->data.jump_statement.kind = JUMP_STATEMENT_CONTINUE;
    } else {
        return ERROR_INTERNAL; // TODO error
    }

    if ((code = parser_expected(p, C_TOKEN_SEMICOLON, &semicolon)) != ERROR_OK) {
        return code;
    }

    result->span.begin = keyword_token.span.begin;
    result->span.end = semicolon.span.end;

    return ERROR_OK;
}

Error parse_compound_statement(Parser *p, Statement *result);

Error parse_statement(Parser *p, Statement *result) {
    const CToken *token = parser_peek(p, 0);

    if (token->kind == C_TOKEN_SEMICOLON) {
        result->kind = STATEMENT_EXPRESSION;
        result->data.expression_statement = nullptr;
        return parser_consume(p, nullptr);
    }

    if ((token->kind == C_TOKEN_IDENTIFIER && parser_peek(p, 1)->kind == C_TOKEN_COLON) ||
        token->kind == C_TOKEN_KW_CASE || token->kind == C_TOKEN_KW_DEFAULT) {
        return parse_labeled_statement(p, result);
    }

    if (token->kind == C_TOKEN_LEFT_BRACE) return parse_compound_statement(p, result);

    if (is_expression_start(token->kind)) {
        Expr *expression = parse_expression(p, 0);
        if (expression == nullptr) return ERROR_NULL_POINTER;

        result->kind = STATEMENT_EXPRESSION;
        result->data.expression_statement = expression;

        return parser_expected(p, C_TOKEN_SEMICOLON, nullptr);
    }

    if (token->kind == C_TOKEN_KW_IF || token->kind == C_TOKEN_KW_SWITCH) {
        return parse_selection_statement(p, result);
    }

    if (token->kind == C_TOKEN_KW_WHILE || token->kind == C_TOKEN_KW_DO
        || token->kind == C_TOKEN_KW_FOR) {
        return parse_iteration_statement(p, result);
    }

    if (token->kind == C_TOKEN_KW_RETURN ||
        token->kind == C_TOKEN_KW_BREAK ||
        token->kind == C_TOKEN_KW_GOTO ||
        token->kind == C_TOKEN_KW_CONTINUE
    ) {
        return parse_jump_statement(p, result);
    }

    return ERROR_OK;
}

Error parse_compound_statement(Parser *p, Statement *result) {
    Error code;
    CToken left_brace;

    if ((code = parser_expected(p, C_TOKEN_LEFT_BRACE, &left_brace)) != ERROR_OK) {
        return code;
    }

    vector_init(&result->data.compound_statement.items, sizeof(BlockItem));

    while (parser_peek(p, 0)->kind != C_TOKEN_RIGHT_BRACE &&
           parser_peek(p, 0)->kind != C_TOKEN_EOF) {
        BlockItem item = {0};
        if (is_declaration_specifier(parser_peek(p, 0)->kind)) {
            item.kind = BLOCK_ITEM_DECLARATION;

            code = parse_declaration_specifiers(p, &item.data.declaration.specifiers);
            if (code != ERROR_OK) {
                vector_destroy(&result->data.compound_statement.items);
                return code;
            }

            code = parse_declaration(p, &item.data.declaration);
            if (code != ERROR_OK) {
                vector_destroy(&result->data.compound_statement.items);
                return code;
            }
        } else {
            item.kind = BLOCK_ITEM_STATEMENT;

            if ((code = parse_statement(p, &item.data.statement)) != ERROR_OK) {
                vector_destroy(&result->data.compound_statement.items);
                return code;
            }
        }

        code = vector_push(&result->data.compound_statement.items, &item);
        if (code != ERROR_OK) {
            statement_destroy(&item.data.statement);
            vector_destroy(&result->data.compound_statement.items);
            return code;
        }
    }

    CToken right_brace;

    if ((code = parser_expected(p, C_TOKEN_RIGHT_BRACE, &right_brace)) != ERROR_OK) {
        vector_destroy(&result->data.compound_statement.items);
        return code;
    }

    result->span = (SourceSpan){
        .begin = left_brace.span.begin,
        .end = right_brace.span.end
    };

    return ERROR_OK;
}

Error parse_function_definition(Parser *p, FunctionDefinition *function) {
    Error code;

    function->body = malloc(sizeof(Statement));

    if (function->body == nullptr) return ERROR_ALLOCATION_FAILED;

    if ((code = parse_compound_statement(p, function->body)) != ERROR_OK) {
        return code;
    }

    return ERROR_OK;
}

Error parse_declaration(Parser *p, Declaration *declaration) {
    Error code;
    Vector init_declarator_list;
    vector_init(&init_declarator_list, sizeof(InitDeclarator));

    const CToken *tok = parser_peek(p, 0);

    do {
        if (tok->kind == C_TOKEN_COMMA) {
            code = parser_consume(p, nullptr);
            if (code != ERROR_OK) return code;
            tok = parser_peek(p, 0);
        }

        if (tok->kind == C_TOKEN_SEMICOLON) break;

        InitDeclarator init_declarator;

        code = parse_declarator(p, &init_declarator.declarator);
        if (code != ERROR_OK) return code;

        tok = parser_peek(p, 0);

        if (tok->kind == C_TOKEN_ASSIGN) {
            code = parser_consume(p, nullptr);
            if (code != ERROR_OK) return code;
            tok = parser_peek(p, 0);

            if (tok->kind == C_TOKEN_LEFT_BRACE) {
                // TODO initializer list bs
            } else {
                init_declarator.initializer = parse_expression(p, BP_ASSIGNMENT); // TODO get assignment expression precedence
            }
        }

        code = vector_push(&init_declarator_list, &init_declarator);
        if (code != ERROR_OK) return code;
    } while (tok->kind == C_TOKEN_COMMA);

    declaration->declarator_count = init_declarator_list.length;
    declaration->declarators = init_declarator_list.data;

    if ((code = parser_expected(p, C_TOKEN_SEMICOLON, nullptr)) != ERROR_OK) return code;
    return ERROR_OK;
}

Error parse_storage_class(Parser *p, DeclarationSpecifiers *spec) {
    const CToken *tok = parser_peek(p, 0);

    if (spec->storage_class != STORAGE_NONE) {
        return ERROR_INTERNAL; // TODO error
    }

    if (tok->kind == C_TOKEN_KW_TYPEDEF) spec->storage_class = STORAGE_TYPEDEF;
    else if (tok->kind == C_TOKEN_KW_EXTERN) spec->storage_class = STORAGE_EXTERN;
    else if (tok->kind == C_TOKEN_KW_STATIC) spec->storage_class = STORAGE_STATIC;
    else if (tok->kind == C_TOKEN_KW_AUTO) spec->storage_class = STORAGE_AUTO;
    else if (tok->kind == C_TOKEN_KW_REGISTER) spec->storage_class = STORAGE_REGISTER;

    return parser_consume(p, nullptr);
}

Error parse_type_qualifier(Parser *p, DeclarationSpecifiers *spec) {
    const CToken *tok = parser_peek(p, 0);

    if (tok->kind == C_TOKEN_KW_CONST) {
        if (spec->type_qualifiers & CTYPE_QUALIFIER_CONST) {
            return ERROR_INTERNAL; // TODO error
        }

        spec->type_qualifiers |= CTYPE_QUALIFIER_CONST;
    } else if (tok->kind == C_TOKEN_KW_RESTRICT) {
        if (spec->type_qualifiers & CTYPE_QUALIFIER_RESTRICT) {
            return ERROR_INTERNAL; // TODO error
        }

        spec->type_qualifiers |= CTYPE_QUALIFIER_RESTRICT;
    } else if (tok->kind == C_TOKEN_KW_VOLATILE) {
        if (spec->type_qualifiers & CTYPE_QUALIFIER_VOLATILE) {
            return ERROR_INTERNAL; // TODO error
        }

        spec->type_qualifiers |= CTYPE_QUALIFIER_VOLATILE;
    }

    return parser_consume(p, nullptr);
}

constexpr TypeSpecifier type_specifier_converter[C_TOKEN_COUNT] = {
    [C_TOKEN_KW_VOID] = TYPE_SPEC_VOID,
    [C_TOKEN_KW_CHAR] = TYPE_SPEC_CHAR,
    [C_TOKEN_KW_SHORT] = TYPE_SPEC_SHORT,
    [C_TOKEN_KW_INT] = TYPE_SPEC_INT,
    [C_TOKEN_KW_LONG] = TYPE_SPEC_LONG,
    [C_TOKEN_KW_FLOAT] = TYPE_SPEC_FLOAT,
    [C_TOKEN_KW_DOUBLE] = TYPE_SPEC_DOUBLE,
    [C_TOKEN_KW_SIGNED] = TYPE_SPEC_SIGNED,
    [C_TOKEN_KW_UNSIGNED] = TYPE_SPEC_UNSIGNED,
    // these enums don't exist that so it is a TODO
    // [C_TOKEN_KW__BOOL] = TYPE_SPEC__BOOL,
    // [C_TOKEN_KW__COMPLEX] = TYPE_SPEC__COMPLEX,
};

Error parse_type_specifier(Parser *p, DeclarationSpecifiers *spec) {
    const CToken *tok = parser_peek(p, 0);

    const TypeSpecifier specifier = type_specifier_converter[tok->kind];

    if (spec->type_specifiers & specifier &&
        (spec->type_specifiers & TYPE_SPEC_LONG && spec->long_count >= 2) != specifier
    ) {
        return ERROR_INTERNAL; // TODO error
    }

    spec->type_specifiers |= specifier;

    if (spec->type_specifiers & TYPE_SPEC_LONG) spec->long_count++;

    return parser_consume(p, nullptr);
}

Error parse_declaration_specifiers(Parser *p, DeclarationSpecifiers *result) {
    DeclarationSpecifiers spec = {
        .storage_class = STORAGE_NONE,
        .type_qualifiers = 0,
        .type_specifiers = 0,
        .long_count = 0
    };

    while (true) {
        const CToken *tok = parser_peek(p, 0);
        Error code;

        if (is_storage_class_specifier(tok->kind)) {
            // parse storage class
            code = parse_storage_class(p, &spec);
            if (code != ERROR_OK) return code;
            continue;
        }

        if (is_type_qualifier(tok->kind)) {
            // parse type qualifier
            code = parse_type_qualifier(p, &spec);
            if (code != ERROR_OK) return code;
            continue;
        }

        if (is_type_specifier(tok->kind)) {
            // parse type specifier
            code = parse_type_specifier(p, &spec);
            if (code != ERROR_OK) return code;
            continue;
        }

        break;
    }

    // validate declaration specifiers
    *result = spec;
    return ERROR_OK;
}

static void declarator_insert_inner(Declarator **root, Declarator *node) {
    Declarator **slot = root;

    while ((*slot)->kind != DECL_IDENTIFIER) {
        switch ((*slot)->kind) {
            case DECL_POINTER:
                slot = &(*slot)->pointer.inner;
                break;

            case DECL_ARRAY:
                slot = &(*slot)->array.inner;
                break;

            case DECL_FUNCTION:
                slot = &(*slot)->function.inner;
                break;

            default:
                // impossible
                break;
        }
    }

    node->pointer.inner = *slot; // conceptually node->inner
    *slot = node;
}

Error parse_pointer_prefix(Parser *p, Vector *pointer_list) {
    const CToken *tok = parser_peek(p, 0);
    vector_init(pointer_list, sizeof(Declarator *));

    while (tok->kind == C_TOKEN_ASTERISK) {
        Declarator *declarator = malloc(sizeof(Declarator));
        Error code = parser_consume(p, nullptr); // consume *
        if (code != ERROR_OK) return code;

        declarator->kind = DECL_POINTER;

        {
            DeclarationSpecifiers spec = {0};
            while (is_type_qualifier(tok->kind)) {
                code = parse_type_qualifier(p, &spec);
                if (code != ERROR_OK) return code;
            }
            declarator->pointer.qualifiers = spec.type_qualifiers;
        }

        code = vector_push(pointer_list, &declarator);
        if (code != ERROR_OK) return code;
        tok = parser_peek(p, 0);
    }

    return ERROR_OK;
}

Error parse_declarator_suffix(Parser *p, Declarator **result) {
    const CToken *tok = parser_peek(p, 0);
    Error code;

    Declarator *decl = malloc(sizeof(Declarator));
    if (decl == nullptr) return ERROR_ALLOCATION_FAILED;
    memset(decl, 0, sizeof(Declarator));

    if (tok->kind == C_TOKEN_LEFT_PAREN) {
        // this is a function... thingy. BRING IN THE ASSAULT RIFLE.
        decl->kind = DECL_FUNCTION;

        code = parser_consume(p, nullptr);
        if (code != ERROR_OK) return code;
        tok = parser_peek(p, 0);

        Vector parameters;
        vector_init(&parameters, sizeof(ParameterDeclaration));

        do {
            if (tok->kind == C_TOKEN_COMMA) {
                code = parser_consume(p, nullptr);
                if (code != ERROR_OK) return code;
                tok = parser_peek(p, 0);
            }

            ParameterDeclaration parameter;
            if (tok->kind == C_TOKEN_ELLIPSIS) {
                decl->function.variadic = true;

                if (tok->kind != C_TOKEN_LEFT_PAREN) {
                    return ERROR_INTERNAL; // TODO error
                }

                break;
            }

            code = parse_declaration_specifiers(p, &parameter.specifiers);
            if (code != ERROR_OK) return code;
            tok = parser_peek(p, 0);

            if (tok->kind == C_TOKEN_IDENTIFIER) {
                code = parse_declarator(p, &parameter.declarator);
                if (code != ERROR_OK) return code;
            } else {
                // TODO handle abstract declarators
            }

            if (tok->kind == C_TOKEN_RIGHT_PAREN) break;

            code = vector_push(&parameters, &parameter);
            if (code != ERROR_OK) return code;
        } while (tok->kind == C_TOKEN_COMMA);

        code = parser_match(p, C_TOKEN_RIGHT_PAREN);
        if (code != ERROR_OK) return code;

        decl->function.parameter_count = parameters.length;
        decl->function.parameters = parameters.data;
    } else if (tok->kind == C_TOKEN_LEFT_BRACKET) {
        // this is an array; GET EM!!!!
        decl->kind = DECL_ARRAY;

        code = parser_consume(p, nullptr);
        if (code != ERROR_OK) return code;
        tok = parser_peek(p, 0);

        if (tok->kind == C_TOKEN_KW_STATIC) {
            decl->array.is_static = true;
            code = parser_consume(p, nullptr);
            if (code != ERROR_OK) return code;
            tok = parser_peek(p, 0);
        }

        {
            DeclarationSpecifiers spec = {0};
            while (is_type_qualifier(tok->kind)) {
                code = parse_type_qualifier(p, &spec);
                if (code != ERROR_OK) return code;
            }
            decl->array.qualifiers = spec.type_qualifiers;
        }

        if (tok->kind == C_TOKEN_KW_STATIC && !decl->array.is_static) {
            decl->array.is_static = true;
            code = parser_consume(p, nullptr);
            if (code != ERROR_OK) return code;
            tok = parser_peek(p, 0);
        } else if (tok->kind == C_TOKEN_KW_STATIC && decl->array.is_static) {
            return ERROR_INTERNAL; // TODO error
        }

        if (tok->kind == C_TOKEN_ASTERISK &&
            parser_peek(p, 1)->kind == C_TOKEN_RIGHT_BRACKET) {
            decl->array.is_star = true;
        } else {
            decl->array.size = parse_expression(p, BP_ASSIGNMENT);
        }

        code = parser_match(p, C_TOKEN_RIGHT_BRACKET);
        if (code != ERROR_OK) return code;
    }

    *result = decl;
    return ERROR_OK;
}

Error parse_declarator(Parser *p, Declarator **result) {
    Declarator *decl;
    Vector pointer_list = {0};

    Error code = parse_pointer_prefix(p, &pointer_list);
    if (code != ERROR_OK) return code;

    const CToken *tok = parser_peek(p, 0);

    if (tok->kind == C_TOKEN_IDENTIFIER) {
        decl = malloc(sizeof(Declarator));
        decl->kind = DECL_IDENTIFIER;
        memcpy(&decl->identifier, tok, sizeof(CToken));
        if ((code = parser_consume(p, nullptr)) != ERROR_OK) return code;
    } else if (tok->kind == C_TOKEN_LEFT_PAREN) {
        parser_consume(p, nullptr);
        code = parse_declarator(p, &decl);
        if (code != ERROR_OK) return code;

        if ((code = parser_match(p, C_TOKEN_RIGHT_PAREN)) != ERROR_OK) return code;
    } else {
        return ERROR_INTERNAL; // TODO error
    }

    tok = parser_peek(p, 0);

    while (tok->kind == C_TOKEN_LEFT_PAREN || tok->kind == C_TOKEN_LEFT_BRACKET) {
        Declarator *suffix;
        code = parse_declarator_suffix(p, &suffix);
        if (code != ERROR_OK) return code;

        tok = parser_peek(p, 0);
        declarator_insert_inner(&decl, suffix);
    }

    for (size_t i = pointer_list.length; i > 0; i--) {
        declarator_insert_inner(
            &decl,
            ((Declarator **)pointer_list.data)[i - 1]
        );
    }

    *result = decl;
    return ERROR_OK;
}

Error parse_external_declaration(Parser *p, TranslationUnit *unit) {
    Error code;
    ExternalDeclaration external_declaration = {0};

    DeclarationSpecifiers specs;
    code = parse_declaration_specifiers(p, &specs);
    if (code != ERROR_OK) return code;

    Declarator *declarator;
    code = parse_declarator(p, &declarator);
    if (code != ERROR_OK) return code;

    if (declarator->kind == DECL_FUNCTION &&
        parser_peek(p, 0)->kind == C_TOKEN_LEFT_BRACE) {
        external_declaration.kind = EXTERNAL_DECLARATION_FUNCTION_DEFINITION;
        external_declaration.data.function_definition.specifiers = specs;
        external_declaration.data.function_definition.declarator = *declarator;
        free(declarator);

        code = parse_function_definition(p, &external_declaration.data.function_definition);
        if (code != ERROR_OK) return code;
    } else {
        external_declaration.kind = EXTERNAL_DECLARATION_DECLARATION;
        Declaration *declaration = &external_declaration.data.declaration;
        declaration->specifiers = specs;

        InitDeclarator init_declarator = {0};
        init_declarator.declarator = declarator;

        const CToken *tok = parser_peek(p, 0);

        if (tok->kind == C_TOKEN_ASSIGN) {
            code = parser_consume(p, nullptr);
            if (code != ERROR_OK) return code;
            tok = parser_peek(p, 0);

            if (tok->kind == C_TOKEN_LEFT_BRACE) {
                // TODO initializer list bs
            } else {
                init_declarator.initializer = parse_expression(p, BP_ASSIGNMENT);
            }
        }

        code = parse_declaration(p, &external_declaration.data.declaration);
        if (code != ERROR_OK) return code;

        declaration->declarator_count++;
        InitDeclarator *tmp = realloc(declaration->declarators, declaration->declarator_count * sizeof(InitDeclarator));
        if (tmp == nullptr) return ERROR_ALLOCATION_FAILED;
        declaration->declarators = tmp;

        memcpy(&declaration->declarators[declaration->declarator_count-1], &init_declarator, sizeof(InitDeclarator));
    }

    return vector_push(unit, &external_declaration);
}

Error parse_translation_unit(Parser *p, TranslationUnit *unit) {
    Error code;
    while (parser_peek(p, 0)->kind != C_TOKEN_EOF) {
        if ((code = parse_external_declaration(p, unit)) != ERROR_OK) return code;
    }

    return ERROR_OK;
}
