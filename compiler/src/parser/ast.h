#ifndef NA_16_AST_H
#define NA_16_AST_H

#include <stdint.h>

#include "../lexer/source.h"
#include "../type.h"

typedef struct Declaration Declaration;

typedef enum ExprKind {
    EXPR_INTEGER,

    EXPRESSION_COMMA
} ExprKind;

typedef struct Expr Expr;

struct Expr {
    ExprKind kind;
    SourceSpan span;
    const CType *type;

    union {
        struct {
            uint64_t value;
        } integer_constant;

        struct {
            Expr *left;
            Expr *right;
        } comma;
    } data;
};

typedef enum JumpStatementKind {
    JUMP_STATEMENT_GOTO,
    JUMP_STATEMENT_CONTINUE,
    JUMP_STATEMENT_BREAK,
    JUMP_STATEMENT_RETURN
} JumpStatementKind;

typedef struct JumpStatement {
    JumpStatementKind kind;

    union {
        struct {
            char *label;
        } goto_statement;

        struct {
            Expr *expression;
        } return_statement;
    } data;
} JumpStatement;

typedef enum StatementKind {
    STATEMENT_LABELED,
    STATEMENT_EXPRESSION,
    STATEMENT_COMPOUND,
    STATEMENT_SELECTION,
    STATEMENT_ITERATION,
    STATEMENT_JUMP
} StatementKind;

typedef struct CompoundStatement CompoundStatement;

typedef struct Statement {
    StatementKind kind;
    SourceSpan span;

    union {
        JumpStatement jump_statement;
        CompoundStatement *compound_statement;

        /*
         * Other statement representations added incrementally.
         */
    } data;
} Statement;

typedef enum BlockItemKind {
    BLOCK_ITEM_DECLARATION,
    BLOCK_ITEM_STATEMENT
} BlockItemKind;

typedef struct BlockItem {
    BlockItemKind kind;
    SourceSpan span;

    union {
        Declaration *declaration;
        Statement statement;
    } data;
} BlockItem;

typedef struct CompoundStatement {
    SourceSpan span;

    BlockItem *items;
    size_t count;
    size_t capacity;
} CompoundStatement;

typedef struct FunctionDefinition {
    SourceSpan span;

    char *name;
    const CType *return_type;

    CompoundStatement body;
} FunctionDefinition;

typedef enum ExternalDeclarationKind {
    EXTERNAL_DECLARATION_FUNCTION_DEFINITION,
    EXTERNAL_DECLARATION_DECLARATION
} ExternalDeclarationKind;

typedef struct ExternalDeclaration {
    ExternalDeclarationKind kind;
    SourceSpan span;

    union {
        FunctionDefinition function_definition;
        Declaration *declaration;
    } data;
} ExternalDeclaration;

typedef struct TranslationUnit {
    ExternalDeclaration *items;
    size_t count;
    size_t capacity;
} TranslationUnit;

bool translation_unit_init(TranslationUnit *unit);

bool push_external_declaration(TranslationUnit *unit, const ExternalDeclaration *external_declaration);
bool compound_statement_append(CompoundStatement *compound_statement, const BlockItem *item);

void expression_destroy(Expr *expression);
void statement_destroy(const Statement *statement);
void compound_statement_destroy(const CompoundStatement *compound_statement);
void translation_unit_destroy(const TranslationUnit *unit);

#endif //NA_16_AST_H
