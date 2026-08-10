#ifndef NA_16_AST_H
#define NA_16_AST_H

#include <stdint.h>
#include <vector.h>

#include "../lexer/source.h"
#include "../type.h"
#include "../preprocessor/c_token.h"

typedef struct Declaration Declaration;

typedef enum ExprKind {
    EXPR_INTEGER,
    EXPR_IDENTIFIER,

    EXPR_BINARY,
    EXPR_UNARY,

    EXPR_CALL,
    EXPR_SUBSCRIPT,
    EXPR_MEMBER,

    EXPR_ASSIGN,
    EXPR_CONDITIONAL,
    EXPR_CAST,
} ExprKind;

typedef enum BinaryOp {
    
} BinaryOp;

typedef enum UnaryOp {

} UnaryOp;

typedef struct Expr Expr;

typedef struct Expr {
    ExprKind kind;

    union {
        struct {
            i64 value;
        } integer;

        struct {
            CToken name;
        } identifier;

        struct {
            BinaryOp op;
            Expr *left;
            Expr *right;
        } binary;

        struct {
            UnaryOp op;
            Expr *operand;
        } unary;
    };
} Expr;

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

    Vector items;
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

typedef Vector TranslationUnit;

Error expression_destroy(Expr *expression);
Error statement_destroy(const Statement *statement);
Error compound_statement_destroy(CompoundStatement *compound_statement);
Error translation_unit_destroy(TranslationUnit *translation_unit);

#endif //NA_16_AST_H
