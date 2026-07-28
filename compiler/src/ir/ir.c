#include "ir.h"

#include <stdlib.h>
#include <string.h>

bool ir_instruction_append(IRBasicBlock *block, const IRInstruction *instruction) {
    if (block->instruction_count >= block->instruction_capacity) {
        if (block->instruction_capacity == 0) block->instruction_capacity = 8;
        else block->instruction_capacity *= 2;
        IRInstruction *tmp = realloc(block->instructions, sizeof(IRInstruction) * block->instruction_capacity);
        if (tmp == nullptr) {
            return false;
        }
        block->instructions = tmp;
    }

    block->instructions[block->instruction_count++] = *instruction;
    return true;
}

bool ir_block_append(IRFunction *function, const IRBasicBlock *block) {
    if (function->block_count >= function->block_capacity) {
        if(function->block_capacity == 0) function->block_capacity = 8;
        else function->block_capacity *= 2;

        IRBasicBlock *tmp = realloc(function->blocks, sizeof(IRBasicBlock) * function->block_capacity);
        if (tmp == nullptr) {
            return false;
        }
        function->blocks = tmp;
    }

    function->blocks[function->block_count++] = *block;
    return true;
}

bool ir_function_append(IRModule *module, const IRFunction *function) {
    if (module->function_count >= module->function_capacity) {
        module->function_capacity *= 2;
        IRFunction *tmp = realloc(module->functions, sizeof(IRFunction) * module->function_capacity);
        if (tmp == nullptr) {
            return false;
        }
        module->functions = tmp;
    }

    module->functions[module->function_count++] = *function;
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// AST -> IR lowerer
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool lower_compound_statement(CompoundStatement *statement, IRBasicBlock *block);

static bool lower_expression(Expression *expression, IRValue *result) {
    switch (expression->kind) {
        case EXPRESSION_INTEGER_CONSTANT:
            *result = (IRValue){
                .kind = IR_VALUE_INTEGER_CONSTANT,
                .type = expression->type,
                .data.integer_constant = expression->data.integer_constant.value,
            };
            return true;
        case EXPRESSION_COMMA:
            // if (!lower_expression(expression->data.comma.left, )) return false;
            // if (!lower_expression(expression->data.comma.right, )) return false;
            break;
    }

    return false;
}

static bool lower_jump_statement(JumpStatement *statement, IRBasicBlock *block) {
    IRInstruction instruction = {
        .kind = IR_INSTRUCTION_RETURN,
        .source_span = nullptr,
        .data.return_instruction = {
            .has_value = statement->data.return_statement.expression != nullptr
        }
    };

    switch (statement->kind) {
        case JUMP_STATEMENT_RETURN:
            if (!lower_expression(statement->data.return_statement.expression, &instruction.data.return_instruction.value)) return false;
            block->terminated = true;
            return ir_instruction_append(block, &instruction);
    }

    return false;
}

static bool lower_statement(Statement *statement, IRBasicBlock *block) {
    switch (statement->kind) {
        case STATEMENT_COMPOUND:
            if (!lower_compound_statement(statement->data.compound_statement, block)) return false;
            break;
        case STATEMENT_JUMP:
            if (!lower_jump_statement(&statement->data.jump_statement, block)) return false;
            break;
    }

    return true;
}

static bool lower_compound_statement(CompoundStatement *statement, IRBasicBlock *block) {
    for (size_t i = 0; i < statement->count; i++) {
        BlockItem *item = &statement->items[i];

        switch (item->kind) {
            case BLOCK_ITEM_STATEMENT:
                if (!lower_statement(&item->data.statement, block)) return false;
                break;
            case BLOCK_ITEM_DECLARATION:
                /*
                 * TODO: lower declarations
                 */
                return false;
        }
    }

    return true;
}

static bool lower_function_definition(FunctionDefinition *function, IRModule *module) {
    IRFunction func = {
        .name = strdup(function->name),
        .source_span = function->span,
        .return_type = function->return_type,
        .blocks = nullptr
    };

    if (func.name == nullptr) {
        return false;
    }

    {
        const IRBasicBlock entry = {
            .id = 0
        };

        if (!ir_block_append(&func, &entry)) {
            // destroy ir function
            return false;
        }
    }

    IRBasicBlock *entry_block = &func.blocks[func.entry_block];

    if (!lower_compound_statement(&function->body, entry_block)) {
        // destroy ir function
        return false;
    }

    if (!entry_block->terminated) {
        /*
         * TODO:
         * - Implicit return for void functions.
         * - Special handling for reaching the end of main.
         * - Non-void fallthrough behaviour.
         */
        // destroy ir function
        return false;
    }

    return ir_function_append(module, &func);
}

bool lower_ast(TranslationUnit *unit, IRModule *module) {
    for (size_t i = 0; i < unit->count; i++) {
        ExternalDeclaration *item = &unit->items[i];

        if (item->kind == EXTERNAL_DECLARATION_FUNCTION_DEFINITION) {
            if (!lower_function_definition(&item->data.function_definition, module)) return false;
        }
    }

    return true;
}