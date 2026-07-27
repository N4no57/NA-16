#include "ast.h"

#include <stdlib.h>

bool translation_unit_init(TranslationUnit *unit) {
    unit->count = 0;
    unit->capacity = 8;
    unit->items = malloc(sizeof(ExternalDeclaration) * unit->capacity);

    if (!unit->items) return false;
    return true;
}

bool push_external_declaration(TranslationUnit *unit, const ExternalDeclaration *external_declaration) {
    if (unit->count == unit->capacity) {
        unit->capacity *= 2;
        ExternalDeclaration *tmp = realloc(unit->items, sizeof(FunctionDefinition) * unit->capacity);
        if (!tmp) return false;

        unit->items = tmp;
    }

    unit->items[unit->count++] = *external_declaration;

    return true;
}

bool compound_statement_append(CompoundStatement *compound_statement, const BlockItem *item) {
    if (compound_statement->count == compound_statement->capacity) {
        compound_statement->capacity *= 2;
        BlockItem *tmp = realloc(compound_statement->items, sizeof(BlockItem) * compound_statement->capacity);
        if (!tmp) return false;

        compound_statement->items = tmp;
    }

    compound_statement->items[compound_statement->count++] = *item;
    return true;
}

void expression_destroy(Expression *expression) {
    if (expression->kind == EXPRESSION_INTEGER_CONSTANT) {
        free(expression);
    } else if (expression->kind == EXPRESSION_COMMA) {
        expression_destroy(expression->data.comma.left);
        expression_destroy(expression->data.comma.right);
        free(expression);
    }
}

void statement_destroy(const Statement *statement) {
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
}

void compound_statement_destroy(const CompoundStatement *compound_statement) {
    for (size_t i = 0; i < compound_statement->count; i++) {
        const BlockItem *item = &compound_statement->items[i];

        if (item->kind == BLOCK_ITEM_STATEMENT) {
            statement_destroy(&item->data.statement);
        }
    }

    free(compound_statement->items);
}

void translation_unit_destroy(const TranslationUnit *unit) {
    for (size_t i = 0; i < unit->count; i++) {
        const ExternalDeclaration *external_declaration = &unit->items[i];

        if (external_declaration->kind == EXTERNAL_DECLARATION_FUNCTION_DEFINITION) {
            const FunctionDefinition *function_definition = &external_declaration->data.function_definition;
            free(function_definition->name);

            compound_statement_destroy(&function_definition->body);
        }
    }

    free(unit->items);
}
