#include "ast.h"

#include <stdlib.h>

Error expression_destroy(Expr *expression) {
    switch (expression->kind) {
        case EXPR_UNARY: {

        }
        case EXPR_BINARY: {

        }
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

    }

    return ERROR_OK;
}

Error translation_unit_destroy(TranslationUnit *translation_unit) {
    for (size_t i = 0; i < translation_unit->length; i++) {
        const ExternalDeclaration *external_declaration = &((ExternalDeclaration *)translation_unit->data)[i];
    }

    return vector_destroy(translation_unit);
}
