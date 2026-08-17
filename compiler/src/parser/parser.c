#include "parser.h"

#include <stdlib.h>
#include <string.h>

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

Expr *parse_expression(Parser *p, float min_bp) {
    const CToken *tok = parser_peek(p, 0);
    Expr *left = nullptr;

    if (tok->kind != C_TOKEN_LEFT_PAREN) {
        parser_consume(p, nullptr);
        left = parse_expression(p, 0);

        tok = parser_peek(p, 0);
        if (tok->kind != C_TOKEN_RIGHT_PAREN) {
            return nullptr;
        }
        return left;
    }

    left = malloc(sizeof(Expr));

    while (true) {
        tok = parser_peek(p, 0);

        if (
            tok->kind == C_TOKEN_EOF ||
            tok->kind == C_TOKEN_RIGHT_PAREN) {
            break;
        }

        parser_consume(p, nullptr);

        float r_bp = 0;

        Expr *right = parse_expression(p, r_bp);
    }

    return left;
}

Error parse_jump_statement(Parser *p, Statement *result) {
    Error code;
    CToken return_token;

    if ((code = parser_expected(p, C_TOKEN_KW_RETURN, &return_token)) != ERROR_OK) {
        return code;
    }

    Expr *expression = nullptr;

    if (parser_peek(p, 0)->kind != C_TOKEN_SEMICOLON) {
        expression = parse_expression(p, 0);

        if (expression == nullptr) {
            return ERROR_NULL_POINTER;
        }
    }

    CToken semicolon;

    if ((code = parser_expected(p, C_TOKEN_SEMICOLON, &semicolon)) != ERROR_OK) {
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

Error parse_compound_statement(Parser *p, Statement *result);

Error parse_statement(Parser *p, Statement *result) {
    const CToken *token = parser_peek(p, 0);

    if (token->kind == C_TOKEN_LEFT_BRACE) return parse_compound_statement(p, result);

    if (token->kind == C_TOKEN_LEFT_PAREN ||
        token->kind == C_TOKEN_IDENTIFIER ||
        token->kind == C_TOKEN_CHARACTER_CONSTANT ||
        token->kind == C_TOKEN_FLOATING_CONSTANT ||
        token->kind == C_TOKEN_INTEGER_CONSTANT ||
        token->kind == C_TOKEN_STRING_LITERAL
    ) {
        Expr *expression = parse_expression(p, 0);
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
        BlockItem item;
        item.kind = BLOCK_ITEM_STATEMENT;

        if ((code = parse_statement(p, &item.data.statement)) != ERROR_OK) {
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

    {
        CToken function_name;

        if ((code = parser_expected(p, C_TOKEN_IDENTIFIER, &function_name)) != ERROR_OK) {
            return code;
        }

        function->declarator.identifier = function_name;
    }

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
    parser_consume(p, nullptr);
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
        if (spec->type_qualifiers & C_TOKEN_KW_RESTRICT) {
            return ERROR_INTERNAL; // TODO error
        }

        spec->type_qualifiers |= C_TOKEN_KW_RESTRICT;
    } else if (tok->kind == C_TOKEN_KW_VOLATILE) {
        if (spec->type_qualifiers & C_TOKEN_KW_VOLATILE) {
            return ERROR_INTERNAL; // TODO error
        }

        spec->type_qualifiers |= C_TOKEN_KW_VOLATILE;
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
};

Error parse_type_specifier(Parser *p, DeclarationSpecifiers *spec) {
    const CToken *tok = parser_peek(p, 0);

    const TypeSpecifier specifier = type_specifier_converter[tok->kind];

    if (spec->type_specifiers & specifier &&
        (spec->type_qualifiers & TYPE_SPEC_LONG && spec->long_count >= 2) != specifier
    ) {
        return ERROR_INTERNAL; // TODO error
    }

    spec->type_specifiers |= specifier;

    if (spec->type_qualifiers & TYPE_SPEC_LONG) spec->long_count++;

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
                code = parse_type_specifier(p, &spec);
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

Error parse_declarator(Parser *p, Declarator **result);

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
                code = parse_type_specifier(p, &spec);
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
        external_declaration.data.declaration.specifiers = specs;

        code = parse_declaration(p, &external_declaration.data.declaration);
        if (code != ERROR_OK) return code;
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
