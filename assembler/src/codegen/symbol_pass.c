#include <stdlib.h>
#include <string.h>

#include "codegen.h"
#include "../lib/error.h"

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Symbol table generation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void visit_NodeOperand1(NodeOperand *operand, bool use_16bits, u64 *byte_pos) {
    if (operand->kind == REGISTER || operand->kind == REG_INDIRECT) {
        return;
    }

    if (operand->kind == SYMBOL) {
        return;
    }

    if (operand->kind == IMMEDIATE) {
        if (!use_16bits) {
            (*byte_pos)++;
        } else {
            *byte_pos += 2;
        }
    } else {
        error(operand->pos, "Invalid operand type");
    }
}

void visit_NodeInstruction1(NodeInstruction *node, SymbolTable *table, u64 *byte_pos) {
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

    bool use_16bits = false;
    if (node->operand_size == 0) {
        for (i32 i = 0; i < sig->operand_count; i++) {
            if (sig->kinds[i] == IMMEDIATE) {
                use_16bits = node->operands[i].immediate > 255 ? true : false;
                if (use_16bits) break;
            }
        }
    } else {
        if (node->operand_size == 2) {
            use_16bits = true;
        }
    }

    if (use_16bits) {
        (*byte_pos)++;
    }

    *byte_pos += 2; // instruction encoding

    for (int i = 0; i < node->operand_count; i++) {
        visit_NodeOperand1(&node->operands[i], use_16bits, byte_pos);
    }
}

void visit_NodeSymbol1(NodeSymbol *node, SymbolTable *table, const u64 *byte_pos) {
    NodeSymbol *symbol = find_symbol(table, node->symbol_name);

    if (symbol) {
        error(node->pos, "Reused symbol");
    }

    if (node->kind == SK_LABEL) { // it's a label
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Symbol table correction
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void visit_NodeOperandRecalc(NodeOperand *operand, SymbolTable *table, bool use_16bits, u64 *byte_pos) {
    if (operand->kind == REGISTER || operand->kind == REG_INDIRECT) {
        return;
    }

    if (operand->kind == SYMBOL) {
        NodeSymbol *symbol = find_symbol(table, operand->symbol_name);

        u8 inc_by = 0;
        if (!use_16bits && symbol->value > 255) inc_by += 2; // account for new prefix and extra byte for the symbol
        else if (use_16bits) inc_by++;
        inc_by++;

        for (u64 i = 0; i < table->count; i++) {
            NodeSymbol *sym = &table->symbols[i];
            if (sym->kind == SK_LABEL && sym->value >= *byte_pos) {
                sym->value += inc_by;
            }
        }

        *byte_pos += inc_by;
        return;
    }

    if (operand->kind == IMMEDIATE) {
        if (!use_16bits) {
            (*byte_pos)++;
        } else {
            *byte_pos += 2;
        }
    } else {
        error(operand->pos, "Invalid operand type");
    }
}

void visit_NodeInstructionRecalc(NodeInstruction *node, SymbolTable *table, u64 *byte_pos) {
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

    bool use_16bits = false;
    if (node->operand_size == 0) {
        for (i32 i = 0; i < sig->operand_count; i++) {
            if (sig->kinds[i] == IMMEDIATE) {
                use_16bits = node->operands[i].immediate > 255 ? true : false;
                if (use_16bits) break;
            }
        }
    } else {
        if (node->operand_size == 2) {
            use_16bits = true;
        }
    }

    if (use_16bits) {
        (*byte_pos)++;
    }

    *byte_pos += 2; // instruction encoding

    for (int i = 0; i < node->operand_count; i++) {
        visit_NodeOperandRecalc(&node->operands[i], table, use_16bits, byte_pos);
    }
}

void visit_NodeStatementRecalc(NodeStatement *node, SymbolTable *table, u64 *byte_pos) {
    if (node->kind == ST_INSTRUCTION) {
        visit_NodeInstructionRecalc(&node->instruction, table, byte_pos);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Symbol table usage
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void visit_NodeOperand2(NodeOperand *operand, SymbolTable *table) {
    if (operand->kind == REGISTER || operand->kind == REG_INDIRECT) {
        return;
    }

    if (operand->kind == SYMBOL) {
        NodeSymbol *symbol = find_symbol(table, operand->symbol_name);

        operand->kind = IMMEDIATE;
        operand->immediate = symbol->value;
        return;
    }

    if (operand->kind == IMMEDIATE) {
        return;
    }

    error(operand->pos, "Invalid operand type");
}

void visit_NodeInstruction2(NodeInstruction *node, SymbolTable *table, u64 *byte_pos) {
    for (int i = 0; i < node->operand_count; i++) {
        visit_NodeOperand2(&node->operands[i], table);
    }
}

void visit_NodeStatement2(NodeStatement *node, SymbolTable *table, u64 *byte_pos) {
    if (node->kind == ST_INSTRUCTION) {
        visit_NodeInstruction2(&node->instruction, table, byte_pos);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Symbol Pass
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void symbol_pass(NodeProgram *ast, SymbolTable *table) {
    if (!ast) return;

    // generate symbol table
    u64 byte_pos = 0;
    for (u64 i = 0; i < ast->count; i++) {
        visit_NodeStatement1(&ast->statements[i], table, &byte_pos);
    }

    // recalculate and correct symbols
    byte_pos = 0;
    for (u64 i = 0; i < ast->count; i++) {
        visit_NodeStatementRecalc(&ast->statements[i], table, &byte_pos);
    }

    // replace symbols with values
    byte_pos = 0;
    for (u64 i = 0; i < ast->count; i++) {
        visit_NodeStatement2(&ast->statements[i], table, &byte_pos);
    }
}