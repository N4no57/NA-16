#include <stdlib.h>
#include <string.h>

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

NodeSymbol *find_symbol(SymbolTable *table, char *symbol) {
    for (u64 i = 0; i < table->count; i++) {
        if (strcmp(table->symbols[i].symbol_name, symbol) == 0) {
            return &table->symbols[i];
        }
    }

    return nullptr;
}

void visit_NodeOperand1(const NodeOperand *operand, bool use_16bits);

void visit_NodeInstruction1(const NodeInstruction *node, SymbolTable *table, u64 *byte_pos) {
    InstructionSpec info = get_spec(node->mnemonic);

    u64 sig_id = 0;
    InstructionSignature *sig = &info.signatures[sig_id];
    for (sig_id = 0; sig_id < info.signature_count; sig_id++) {
        if (match_signature(node, sig)) break;
        sig = &info.signatures[sig_id+1];
    }

    if (sig_id > 0) {
        *byte_pos += 2;
    }
}

void visit_NodeSymbol1(NodeSymbol *node, SymbolTable *table, const u64 *byte_pos) {
    NodeSymbol *symbol = find_symbol(table, node->symbol_name);

    if (symbol) {
        error(node->pos, "Reused symbol");
    }

    if (node->value == -1) { // it's a label
        node->value = *(i32 *)byte_pos;
    }

    push_symbol(table, *node);
}

void visit_NodeStatement1(NodeStatement *node, SymbolTable *table, u64 *byte_pos) {
    if (node->kind == ST_INSTRUCTION) {
        visit_NodeInstruction1(&node->instruction, table, byte_pos);
    } else if (node->kind == ST_SYMBOL) {
        visit_NodeSymbol1(&node->symbol, table, byte_pos);
    }
}

void symbol_pass(NodeProgram *ast, SymbolTable *table) {
    if (!ast) return;

    for (u64 i = 0; i < ast->count; i++) {
        Visit
    }
}