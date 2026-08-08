#include "ast.h"

#include <stdlib.h>


Error expression_destroy(Expr *expression) {
    if (expression->kind == EXPR_INTEGER) {
        free(expression);
    } else if (expression->kind == EXPRESSION_COMMA) {
        expression_destroy(expression->data.comma.left);
        expression_destroy(expression->data.comma.right);
        free(expression);
    }

    return ERROR_OK;
}

Error statement_destroy(const Statement *statement) {
    if (statement->kind == STATEMENT_JUMP) {
        const JumpStatement *jump_statement = &statement->data.jump_statement;

        if (jump_statement->kind == JUMP_STATEMENT_RETURN) {
            expression_destroy(jump_statement->data.return_statement.expression);
        } else if (jump_statement->kind == JUMP_STATEMENT_GOTO) {
            free(jump_statement->data.goto_statement.label);
        }
    } else if (statement->kind == STATEMENT_COMPOUND) {
        compound_statement_destroy(statement->data.compound_statement);
    }

    return ERROR_OK;
}

Error compound_statement_destroy(CompoundStatement *compound_statement) {
    for (size_t i = 0; i < compound_statement->items.length; i++) {
        const BlockItem *item = &((BlockItem *)compound_statement->items.data)[i];

        if (item->kind == BLOCK_ITEM_STATEMENT) {
            const Error code = statement_destroy(&item->data.statement);
            if (code != ERROR_OK) return code;
        }
    }

    return vector_destroy(&compound_statement->items);
}

Error translation_unit_destroy(TranslationUnit *translation_unit) {
    for (size_t i = 0; i < translation_unit->length; i++) {
        const ExternalDeclaration *external_declaration = &((ExternalDeclaration *)translation_unit->data)[i];

        if (external_declaration->kind == EXTERNAL_DECLARATION_FUNCTION_DEFINITION) {
            FunctionDefinition *function_definition = &external_declaration->data.function_definition;
            free(function_definition->name);

            compound_statement_destroy(&function_definition->body);
        }
    }

    return vector_destroy(translation_unit);
}
