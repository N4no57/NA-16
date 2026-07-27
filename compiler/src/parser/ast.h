#ifndef NA_16_AST_H
#define NA_16_AST_H

#include <stdint.h>

#include "../lexer/source.h"
#include "../type.h"

typedef enum ExpressionKind {
    EXPRESSION_INTEGER_CONSTANT
} ExpressionKind;

typedef struct Expression {
    ExpressionKind kind;
    SourceSpan span;
    const CType *type;

    union {
        struct {
            uint64_t value;
        } integer_constant;
    } data;
} Expression;

typedef enum StatementKind {
    STATEMENT_RETURN
} StatementKind;

typedef struct Statement {
    StatementKind kind;
    SourceSpan span;

    union {
        struct {
            Expression *expression;
        } return_statement;
    } data;
} Statement;

typedef struct FunctionDefinition {
    SourceSpan span;

    char *name;
    const CType *return_type;

    Statement *body;
} FunctionDefinition;

typedef struct TranslationUnit {
    FunctionDefinition *functions;
    size_t function_count;
    size_t function_capacity;
} TranslationUnit;

void translation_unit_init(TranslationUnit *unit);

void push_function(TranslationUnit *unit, const FunctionDefinition *function);

void translation_unit_destroy(const TranslationUnit *unit);

#endif //NA_16_AST_H
