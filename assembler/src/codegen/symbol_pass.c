#include <stdlib.h>

#include "codegen.h"

void push_symbol(SymbolTable *table, NodeSymbol symbol) {
    if (table->count >= table->size) {
        if (table->size == 0) table->size = 256;
        else table->size *= 2;
        NodeSymbol *tmp = realloc(table->symbols, table->size * sizeof(NodeSymbol));
        if (tmp == nullptr) {
            exit(EXIT_FAILURE);
        }
        table->symbols = tmp;
    }

    table->symbols[table->count++] = symbol;
}

void visit_NodeOperand(const NodeOperand *operand, bool use_16bits, u8 *buff, u8 *idx);

void visit_NodeInstruction(const NodeInstruction *node, bytes *code);

void visit_NodeStatement(const NodeStatement *node, bytes *code);

void symbol_pass(NodeProgram *ast, SymbolTable *table) {
    if (!ast) return;

    for (u64 i = 0; i < ast->count; i++) {
        Visit
    }
}