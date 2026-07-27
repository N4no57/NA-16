#include "analyser.h"

bool analyse_statement(SemanticContext *context, const Statement *statement);
bool analyse_compound_statement(SemanticContext *context, const CompoundStatement *compound);

bool analyse_statement(SemanticContext *context, const Statement *statement) {
    if (statement->kind == STATEMENT_COMPOUND) {
        if (!analyse_compound_statement(context, statement->data.compound_statement)) {
            return false;
        }
    } else if (statement->kind == STATEMENT_JUMP) {
        const JumpStatement *jump_statement = &statement->data.jump_statement;

        if (jump_statement->kind == JUMP_STATEMENT_RETURN) {
            const CType *type = jump_statement->data.return_statement.expression->type;
            return type->kind == context->current_function_return_type->kind;
        }
    }

    return true;
}

bool analyse_compound_statement(SemanticContext *context, const CompoundStatement *compound) {
    for (size_t i = 0; i < compound->count; i++) {
        const BlockItem *item = &compound->items[i];

        if (item->kind == BLOCK_ITEM_STATEMENT) {
            if (!analyse_statement(context, &item->data.statement)) {
                return false;
            }
        }
    }

    return true;
}

bool analyse_function_definition(SemanticContext *context, const FunctionDefinition *function) {
    context->current_function_return_type = function->return_type;

    analyse_compound_statement(context, &function->body);

    return true;
}
