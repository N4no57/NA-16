#include "parser.h"

#include <stdlib.h>
#include <string.h>

void parser_init(Parser *parser, TokenStream *tokens) {
    parser->tokens = tokens;
    memset(&parser->error, 0, sizeof(parser->error));
}

static const CToken *parser_peek(const Parser *parser) {
    return token_stream_peek(parser->tokens);
}

static Error parser_consume(const Parser *parser) {
    return token_stream_consume(parser->tokens);
}

static Error parser_match(const Parser *parser, const CTokenKind expected) {
    return token_stream_match(parser->tokens, expected);
}

static Error parser_expected(Parser *parser, const CTokenKind expected, CToken *result) {
    const CToken *token = parser_peek(parser);

    if (token == nullptr) {
        return ERROR_NULL_POINTER;
    }

    if (token->kind != expected) {
        parser->error.span = token->span;
        parser->error.message = "Unexpected token";

        return ERROR_NOT_FOUND;
    }

    if (result != nullptr) {
        *result = *token;
    }

    return token_stream_consume(parser->tokens);
}

Expression *parser_parse_assignment_expression(const Parser *parser) {
    const CToken *token = parser_peek(parser);

    if (token == nullptr || token->kind != C_TOKEN_INTEGER_CONSTANT) {
        return nullptr;
    }

    Expression *expression = malloc(sizeof(Expression));

    if (expression == nullptr) {
        return nullptr;
    }

    const CType *type = ctype_builtin(CTYPE_INT);

    *expression = (Expression){
        .kind = EXPRESSION_INTEGER_CONSTANT,
        .span = token->span,
        .type = type,
        .data.integer_constant = {
            .value = token->data.integer.unsigned_int
        }
    };

    parser_consume(parser);
    return expression;
}

Expression *parser_parse_expression(Parser *parser) {
    /*
     * expression ::= assignment-expression
     *              | expression ',' assignment-expression
     */

    Expression *left = parser_parse_assignment_expression(parser);

    if (left == nullptr) {
        return nullptr;
    }

    while (parser_match(parser, C_TOKEN_COMMA) == ERROR_OK) {
        Expression *right = parser_parse_assignment_expression(parser);

        if (right == nullptr) {
            free(left);
            return nullptr;
        }

        Expression *comma = malloc(sizeof(*comma));

        if (comma == nullptr) {
            free(left);
            free(right);
            return nullptr;
        }

        *comma = (Expression){
            .kind = EXPRESSION_COMMA,
            .span = {
                .begin = left->span.begin,
                .end = right->span.end,
            },
            .type = right->type,
            .data.comma = {
                .left = left,
                .right = right
            }
        };

        left = comma;
    }

    return left;
}

Error parser_parse_jump_statement(Parser *parser, Statement *result) {
    Error code;
    CToken return_token;

    if ((code = parser_expected(parser, C_TOKEN_KW_RETURN, &return_token)) != ERROR_OK) {
        return code;
    }

    Expression *expression = nullptr;

    if (parser_peek(parser)->kind != C_TOKEN_SEMICOLON) {
        expression = parser_parse_expression(parser);

        if (expression == nullptr) {
            return ERROR_NULL_POINTER;
        }
    }

    CToken semicolon;

    if ((code = parser_expected(parser, C_TOKEN_SEMICOLON, &semicolon)) != ERROR_OK) {
        expression_destroy(expression);
        return code;
    }

    *result = (Statement){
        .kind = STATEMENT_JUMP,
        .span = {
            .begin = return_token.span.begin,
            .end = semicolon.span.end
        },
        .data.jump_statement = {
            .kind = JUMP_STATEMENT_RETURN,
            .data.return_statement.expression = expression
        }
    };

    return ERROR_OK;
}

Error parser_parse_statement(Parser *parser, Statement *result) {
    const CToken *token = parser_peek(parser);

    if (token->kind == C_TOKEN_KW_RETURN) {
        return parser_parse_jump_statement(parser, result);
    }

    parser->error.span = parser_peek(parser)->span;
    parser->error.message = "Expected \"return\"";

    return ERROR_OK;
}

Error parser_parse_compound_statement(Parser *parser, CompoundStatement *result) {
    Error code;
    CToken left_brace;

    if ((code = parser_expected(parser, C_TOKEN_LEFT_BRACE, &left_brace)) != ERROR_OK) {
        return code;
    }

    CompoundStatement compound = {0};
    compound.capacity = 8;
    compound.items = malloc(sizeof(BlockItem) * compound.capacity);

    while (parser_peek(parser)->kind != C_TOKEN_RIGHT_BRACE &&
           parser_peek(parser)->kind != C_TOKEN_EOF) {
        BlockItem item;
        item.kind = BLOCK_ITEM_STATEMENT;

        if ((code = parser_parse_statement(parser, &item.data.statement)) != ERROR_OK) {
            compound_statement_destroy(&compound);
            return code;
        }

        if ((code = compound_statement_append(&compound, &item)) != ERROR_OK) {
            statement_destroy(&item.data.statement);
            compound_statement_destroy(&compound);
            return code;
        }
    }

    CToken right_brace;

    if ((code = parser_expected(parser, C_TOKEN_RIGHT_BRACE, &right_brace)) != ERROR_OK) {
        compound_statement_destroy(&compound);
        return code;
    }

    compound.span = (SourceSpan){
        .begin = left_brace.span.begin,
        .end = right_brace.span.end
    };

    *result = compound;
    return ERROR_OK;
}

Error parser_parse_function_definition(Parser *parser, FunctionDefinition *function) {
    Error code;
    CToken declarator;

    if ((code = parser_expected(parser, C_TOKEN_KW_INT, &declarator)) != ERROR_OK) {
        return code;
    }

    function->return_type = ctype_builtin(CTYPE_INT);

    {
        CToken function_name;

        if ((code = parser_expected(parser, C_TOKEN_IDENTIFIER, &function_name)) != ERROR_OK) {
            return code;
        }

        const char *name_start = &function_name.span.begin.file->contents[function_name.span.begin.offset];
        const size_t function_name_size = function_name.span.end.offset - function_name.span.begin.offset;

        function->name = malloc(function_name_size+1);

        memcpy(function->name, name_start, function_name_size);

        function->name[function_name_size] = '\0';
    }

    if ((code = parser_expected(parser, C_TOKEN_LEFT_PAREN, nullptr)) != ERROR_OK) {
        return code;
    }

    token_stream_match(parser->tokens, C_TOKEN_KW_VOID);

    if ((code = parser_expected(parser, C_TOKEN_RIGHT_PAREN, nullptr)) != ERROR_OK) {
        return code;
    }

    if ((code = parser_parse_compound_statement(parser, &function->body)) != ERROR_OK) {
        return code;
    }

    function->span = (SourceSpan){
        .begin = declarator.span.begin,
        .end = function->body.span.end
    };

    return ERROR_OK;
}

Error parser_parse_external_declaration(Parser *parser, TranslationUnit *unit) {
    Error code;
    ExternalDeclaration external_declaration;
    external_declaration.kind = EXTERNAL_DECLARATION_FUNCTION_DEFINITION;
    if ((code = parser_parse_function_definition(parser, &external_declaration.data.function_definition)) != ERROR_OK) {
        return code;
    }

    push_external_declaration(unit, &external_declaration);

    // TODO: declaration

    return ERROR_OK;
}

Error parser_parse_translation_unit(Parser *parser, TranslationUnit *unit) {
    Error code;
    while (parser_peek(parser)->kind != C_TOKEN_EOF) {
        if ((code = parser_parse_external_declaration(parser, unit))) return code;
    }

    return ERROR_OK;
}
