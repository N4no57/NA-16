#ifndef NA_16_AST_H
#define NA_16_AST_H

#include <stdint.h>
#include <vector.h>

#include "../lexer/source.h"
#include "../preprocessor/c_token.h"

typedef enum ExprKind {
    EXPR_CONSTANT,
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
    BIN_OP_MULTIPLY,
    BIN_OP_DIVIDE,
    BIN_OP_MOD,
    BIN_OP_ADD,
    BIN_OP_SUBTRACT,
    BIN_OP_SHIFT_LEFT,
    BIN_OP_SHIFT_RIGHT,
    BIN_OP_LESS,
    BIN_OP_GREATER,
    BIN_OP_LESS_OR_EQUAL,
    BIN_OP_GREATER_OR_EQUAL,
    BIN_OP_EQUAL,
    BIN_OP_NOT_EQUAL,
    BIN_OP_BITWISE_AND,
    BIN_OP_EXCLUSIVE_OR,
    BIN_OP_INCLUSIVE_OR,
    BIN_OP_LOGICAL_AND,
    BIN_OP_LOGICAL_OR,
    BIN_OP_COMMA
} BinaryOp;

typedef enum UnaryOp {
    OP_PREFIX_INCREMENT,
    OP_PREFIX_DECREMENT,
    OP_POSTFIX_INCREMENT,
    OP_POSTFIX_DECREMENT,
    OP_AMPERSAND,
    OP_ASTERISK,
    OP_PLUS,
    OP_MINUS,
    OP_BITWISE_NOT,
    OP_LOGICAL_NOT,
    OP_CAST
} UnaryOp;

typedef enum ConstantKind {
    CONSTANT_INTEGER,
    CONSTANT_FLOAT,
    CONSTANT_CHAR,
    CONSTANT_STRING_LITERAL
} ConstantKind;

typedef struct Expr Expr;
typedef struct Expr {
    ExprKind kind;

    union {
        struct {
            ConstantKind kind;

            union {
                struct {
                    union {
                        uint64_t unsigned_int;
                        int64_t signed_int;
                    };
                    IntegerSuffix suffix;
                } integer;

                double floating;
                char *character_or_str;
            };
        } constant;

        struct {
            CToken name;
        } identifier;

        struct {
            BinaryOp op;
            Expr *left;
            Expr *right;

            CToken op_tok; // To lazy to add assigns for each kind so this is here
        } binary;

        struct {
            UnaryOp op;
            Expr *operand;
        } unary;

        struct {
            Expr *expression;
            Expr *true_expression;
            Expr *false_expression;
        } conditional;

        struct {
            Expr **args;
            size_t argument_count;
            Expr *name;
        } function_call;

        struct {
            bool dereference;
            Expr *member;
            CToken member_item;
        } member;
    };
} Expr;

typedef enum StorageClass {
    STORAGE_NONE,
    STORAGE_TYPEDEF,
    STORAGE_EXTERN,
    STORAGE_STATIC,
    STORAGE_AUTO,
    STORAGE_REGISTER
} StorageClass;

typedef enum TypeSpecifier {
    TYPE_SPEC_VOID     = 1 << 0,
    TYPE_SPEC_CHAR     = 1 << 1,
    TYPE_SPEC_SHORT    = 1 << 2,
    TYPE_SPEC_INT      = 1 << 3,
    TYPE_SPEC_LONG     = 1 << 4,
    TYPE_SPEC_FLOAT    = 1 << 5,
    TYPE_SPEC_DOUBLE   = 1 << 6,
    TYPE_SPEC_SIGNED   = 1 << 7,
    TYPE_SPEC_UNSIGNED = 1 << 8,
} TypeSpecifier;

typedef struct DeclarationSpecifiers {
    StorageClass storage_class;

    unsigned type_qualifiers;
    unsigned type_specifiers;

    unsigned char long_count;
} DeclarationSpecifiers;

typedef enum DeclaratorKind {
    DECL_IDENTIFIER,
    DECL_POINTER,
    DECL_ARRAY,
    DECL_FUNCTION,
    DECL_ABSTRACT
} DeclaratorKind;

typedef struct Declarator Declarator;

typedef struct ParameterDeclaration {
    DeclarationSpecifiers specifiers;
    Declarator *declarator;
} ParameterDeclaration;

typedef struct Declarator {
    DeclaratorKind kind;

    union {
        CToken identifier;

        struct {
            Declarator *inner;
            unsigned qualifiers;
        } pointer;

        struct {
            Declarator *inner;
            Expr *size;

            unsigned qualifiers;
            bool is_static;
            bool is_star;
        } array;

        struct {
            Declarator *inner;

            ParameterDeclaration *parameters;
            size_t parameter_count;

            bool variadic;
        } function;
    };
} Declarator;

typedef struct InitDeclarator {
    Declarator *declarator;
    Expr *initializer;
} InitDeclarator;

typedef struct Declaration {
    DeclarationSpecifiers specifiers;

    InitDeclarator *declarators;
    size_t declarator_count;
} Declaration;

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

typedef enum LabeledStatementKind {
    LABELED_LABEL,
    LABELED_CASE,
    LABELED_DEFAULT
} LabeledStatementKind;

typedef struct Statement Statement;

typedef struct LabeledStatement {
    LabeledStatementKind kind;

    union {
        char *label; // goto label

        Expr *expression; // expression for switch cases
    } data;

    Statement *statement;
} LabeledStatement;

typedef enum SelectionStatementKind {
    SELECTION_IF,
    SELECTION_SWITCH
} SelectionStatementKind;

typedef struct SelectionStatement {
    SelectionStatementKind kind;

    union {
        struct {
            Expr *conditional;
            Statement *then_branch;
            Statement *else_branch; // null of no else
        } if_stmt;

        struct {
            Expr *expression;
            Statement *body;
        } switch_stmt;
    };
} SelectionStatement;

typedef enum IterationStatementKind {
    ITERATION_WHILE,
    ITERATION_FOR
} IterationStatementKind;

typedef struct IterationStatement {
    IterationStatementKind kind;

    union {
        struct {
            bool do_while;
            Expr* condition;

            Statement *body;
        } while_loop;

        struct {
            union {
                Declarator *declarator; // "int i = 0" or smth
                Expr *expr; // "i = 0" or smth
            } expr1;
            Expr *expr2;
            Expr *expr3;

            Statement *body;
        } for_loop;
    };
} IterationStatement;

typedef enum StatementKind {
    STATEMENT_LABELED,
    STATEMENT_COMPOUND,
    STATEMENT_EXPRESSION,
    STATEMENT_SELECTION,
    STATEMENT_ITERATION,
    STATEMENT_JUMP
} StatementKind;

typedef struct Statement {
    StatementKind kind;
    SourceSpan span;

    union {
        LabeledStatement labeled_statement;

        struct {
            Vector items;
        } compound_statement;

        Expr *expression_statement;

        SelectionStatement selection_statement;

        IterationStatement iteration_statement;

        JumpStatement jump_statement;
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
        Declaration declaration;
        Statement statement;
    } data;
} BlockItem;

typedef struct FunctionDefinition {
    DeclarationSpecifiers specifiers;
    Declarator declarator;
    Statement *body;
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
        Declaration declaration;
    } data;
} ExternalDeclaration;

typedef Vector TranslationUnit;

Error expression_destroy(Expr *expression);
Error statement_destroy(const Statement *statement);
Error translation_unit_destroy(TranslationUnit *translation_unit);

#endif //NA_16_AST_H
