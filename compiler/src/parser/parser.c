#include "parser.h"

#include <stdlib.h>
#include <string.h>

void parser_init(Parser *parser, TokenStream *tokens) {
    parser->tokens = tokens;

    vector_init(&parser->errors, sizeof(ParserError));
}

static const CToken *parser_peek(const Parser *parser, size_t lookahead) {
    return token_stream_peek(parser->tokens, lookahead);
}

static Error parser_consume(const Parser *parser, CToken *token) {
    return token_stream_consume(parser->tokens, token);
}

static Error parser_match(const Parser *parser, const CTokenKind expected) {
    return token_stream_match(parser->tokens, expected);
}

static Error parser_expected(Parser *parser, const CTokenKind expected, CToken *result) {
    const CToken *token = parser_peek(parser, 0);

    if (token == nullptr) {
        return ERROR_NULL_POINTER;
    }

    if (token->kind != expected) {
        // parser->error.span = token->span;
        // parser->error.message = "Unexpected token";

        return ERROR_NOT_FOUND;
    }

    if (result != nullptr) {
        *result = *token;
    }

    return token_stream_consume(parser->tokens, nullptr);
}

typedef struct Precedence {
    float lbp;
    float rbp;
    bool valid;
} Precedence;

static Precedence get_precedence(CTokenKind operator) {
    switch (operator) {
        default:
            return (Precedence){
                .lbp = 0,
                .rbp = 0,
                .valid = false
            };
    }
}

Expr *parse_expression(Parser *parser, float min_bp) {
    const CToken *tok = parser_peek(parser, 0);
    Expr *left = nullptr;

    if (tok->kind != C_TOKEN_LEFT_PAREN) {
        parser_consume(parser, nullptr);
        left = parse_expression(parser, 0);

        tok = parser_peek(parser, 0);
        if (tok->kind != C_TOKEN_RIGHT_PAREN) {
            return nullptr;
        }
        return left;
    }

    left = malloc(sizeof(Expr));

    while (true) {
        tok = parser_peek(parser, 0);

        if (
            tok->kind == C_TOKEN_EOF ||
            tok->kind == C_TOKEN_RIGHT_PAREN) {
            break;
        }

        parser_consume(parser, nullptr);

        float r_bp = 0;

        Expr *right = parse_expression(parser, r_bp);
    }

    return left;
}

Error parse_jump_statement(Parser *parser, Statement *result) {
    Error code;
    CToken return_token;

    if ((code = parser_expected(parser, C_TOKEN_KW_RETURN, &return_token)) != ERROR_OK) {
        return code;
    }

    Expr *expression = nullptr;

    if (parser_peek(parser, 0)->kind != C_TOKEN_SEMICOLON) {
        expression = parse_expression(parser, 0);

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

Error parse_compound_statement(Parser *parser, Statement *result);

Error parse_statement(Parser *parser, Statement *result) {
    const CToken *token = parser_peek(parser, 0);

    if (token->kind == C_TOKEN_LEFT_BRACE) return parse_compound_statement(parser, result);

    if (token->kind == C_TOKEN_LEFT_PAREN ||
        token->kind == C_TOKEN_IDENTIFIER ||
        token->kind == C_TOKEN_CHARACTER_CONSTANT ||
        token->kind == C_TOKEN_FLOATING_CONSTANT ||
        token->kind == C_TOKEN_INTEGER_CONSTANT ||
        token->kind == C_TOKEN_STRING_LITERAL
    ) {
        Expr *expression = parse_expression(parser, 0);
        if (expression == nullptr) return ERROR_NULL_POINTER;

        result->kind = STATEMENT_EXPRESSION;
        result->data.expression_statement = expression;

        return ERROR_OK;
    }

    if (token->kind == C_TOKEN_KW_RETURN ||
        token->kind == C_TOKEN_KW_BREAK ||
        token->kind == C_TOKEN_KW_GOTO ||
        token->kind == C_TOKEN_KW_CONTINUE
    ) {
        return parse_jump_statement(parser, result);
    }

    return ERROR_OK;
}

Error parse_compound_statement(Parser *parser, Statement *result) {
    Error code;
    CToken left_brace;

    if ((code = parser_expected(parser, C_TOKEN_LEFT_BRACE, &left_brace)) != ERROR_OK) {
        return code;
    }

    vector_init(&result->data.compound_statement.items, sizeof(BlockItem));

    while (parser_peek(parser, 0)->kind != C_TOKEN_RIGHT_BRACE &&
           parser_peek(parser, 0)->kind != C_TOKEN_EOF) {
        BlockItem item;
        item.kind = BLOCK_ITEM_STATEMENT;

        if ((code = parse_statement(parser, &item.data.statement)) != ERROR_OK) {
            vector_destroy(&result->data.compound_statement.items);
            return code;
        }

        if ((code = vector_push(&result->data.compound_statement.items, &item)) != ERROR_OK) {
            statement_destroy(&item.data.statement);
            vector_destroy(&result->data.compound_statement.items);
            return code;
        }
    }

    CToken right_brace;

    if ((code = parser_expected(parser, C_TOKEN_RIGHT_BRACE, &right_brace)) != ERROR_OK) {
        vector_destroy(&result->data.compound_statement.items);
        return code;
    }

    result->span = (SourceSpan){
        .begin = left_brace.span.begin,
        .end = right_brace.span.end
    };

    return ERROR_OK;
}

Error parse_function_definition(Parser *parser, FunctionDefinition *function) {
    Error code;

    {
        CToken function_name;

        if ((code = parser_expected(parser, C_TOKEN_IDENTIFIER, &function_name)) != ERROR_OK) {
            return code;
        }

        function->declarator.identifier = function_name;
    }

    function->body = malloc(sizeof(Statement));

    if (function->body == nullptr) return ERROR_ALLOCATION_FAILED;

    if ((code = parse_compound_statement(parser, function->body)) != ERROR_OK) {
        return code;
    }

    return ERROR_OK;
}

DeclarationSpecifiers parse_declaration_specifiers(Parser *parser) {
    const CToken *tok = parser_peek(parser, 0);

    
}

Error parse_external_declaration(Parser *parser, TranslationUnit *unit) {
    Error code;
    ExternalDeclaration external_declaration = {0};

    DeclarationSpecifiers specs = parse_declaration_specifiers(parser);

    Declarator declarator; // = parse_declarator(parser);

    if (declarator.kind == DECL_FUNCTION &&
        parser_peek(parser, 0)->kind == C_TOKEN_LEFT_BRACE) {
        external_declaration.kind = EXTERNAL_DECLARATION_FUNCTION_DEFINITION;
        external_declaration.data.function_definition.specifiers = specs;
        external_declaration.data.function_definition.declarator = declarator;

        code = parse_function_definition(parser, &external_declaration.data.function_definition);
        if (code != ERROR_OK) return code;
    } else {

    }

    return vector_push(unit, &external_declaration);
}

Error parse_translation_unit(Parser *parser, TranslationUnit *unit) {
    Error code;
    while (parser_peek(parser, 0)->kind != C_TOKEN_EOF) {
        if ((code = parse_external_declaration(parser, unit))) return code;
    }

    return ERROR_OK;
}
