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

NodeSymbol *find_symbol(const SymbolTable *table, const char *symbol) {
    for (u64 i = 0; i < table->count; i++) {
        if (strcmp(table->symbols[i].symbol_name, symbol) == 0) {
            return &table->symbols[i];
        }
    }

    return nullptr;
}

bool is_stable(SymbolTable *old, SymbolTable *new) {
    bool ret_val = true;
    for (u64 i = 0; i < old->count; i++) {
        if (ret_val == false) return ret_val;
        if (old->symbols[i].kind == SK_LABEL && new->symbols[i].kind == SK_LABEL) {
            ret_val =  old->symbols[i].value == new->symbols[i].value;
        }
    }
    return ret_val;
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

void visit_NodeOperandRecalc(const NodeOperand *operand, const SymbolTable *table, const bool use_16bits, u64 *size) {
    if (operand->kind == REGISTER || operand->kind == REG_INDIRECT) {
        return;
    }

    if (operand->kind == SYMBOL) {
        NodeSymbol *symbol = find_symbol(table, operand->symbol_name);

        u8 inc_by = 0;
        if (!use_16bits && symbol->value > 255) inc_by += 2; // account for new prefix and extra byte for the symbol
        else if (use_16bits) inc_by++;
        inc_by++;

        *size += inc_by;
        return;
    }

    if (operand->kind == IMMEDIATE) {
        if (!use_16bits) {
            (*size)++;
        } else {
            *size += 2;
        }
    } else {
        error(operand->pos, "Invalid operand type");
    }
}

u64 visit_NodeInstructionRecalc(const NodeInstruction *node, const SymbolTable *table) {
    u64 ret_val = 0;
    InstructionSpec info = get_spec(node->mnemonic);

    u64 sig_id = 0;
    InstructionSignature *sig = &info.signatures[sig_id];
    for (sig_id = 0; sig_id < info.signature_count; sig_id++) {
        if (match_signature(node, sig)) break;
        sig = &info.signatures[sig_id+1];
    }

    if (sig_id > 0) {
        ret_val += 2;
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
        ret_val++;
    }

    ret_val += 2; // instruction encoding

    for (int i = 0; i < node->operand_count; i++) {
        visit_NodeOperandRecalc(&node->operands[i], table, use_16bits, &ret_val);
    }

    return ret_val;
}

u64 visit_NodeStatementRecalc(NodeStatement *node, const SymbolTable *table) {
    u64 ret_val = 0;
    if (node->kind == ST_INSTRUCTION) {
        ret_val = visit_NodeInstructionRecalc(&node->instruction, table);
    }
    return ret_val;
}

void recalc_layout(const NodeProgram *ast, const SymbolTable *old, SymbolTable *new) {
    new->count = old->count;
    new->size = old->size;
    new->symbols = malloc(new->size * sizeof(NodeSymbol));
    memcpy(new->symbols, old->symbols, old->size * sizeof(NodeSymbol));

    u64 *sizes = malloc(ast->count * sizeof(u64));

    for (u64 i = 0; i < ast->count; i++) {
        sizes[i] = visit_NodeStatementRecalc(&ast->statements[i], new);
    }

    u64 byte_pos = 0;

    for (u64 i = 0; i < ast->count; i++) {
        if (ast->statements[i].kind == ST_SYMBOL && ast->statements[i].symbol.kind == SK_LABEL) {
            NodeSymbol *symbol = find_symbol(new, ast->statements[i].symbol.symbol_name);
            symbol->value = (i32)byte_pos;
        }

        byte_pos += sizes[i];
    }

    free(sizes);
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

void symbol_pass(NodeProgram *ast) {
    if (!ast) return;

    SymbolTable old = {0}, new = {0};
    SymbolTable *old_ptr = &old, *new_ptr = &new;

    // generate symbol table
    u64 byte_pos = 0;
    for (u64 i = 0; i < ast->count; i++) {
        visit_NodeStatement1(&ast->statements[i], &old, &byte_pos);
    }

    // recalculate and correct symbols
    do {
        recalc_layout(ast, old_ptr, new_ptr);
        if (is_stable(old_ptr, new_ptr)) break;

        free(old_ptr->symbols);

        // swap
        SymbolTable *tmp = old_ptr;
        old_ptr = new_ptr;
        new_ptr = tmp;
    } while (true);
    free(old_ptr->symbols);
    old_ptr = new_ptr;

    // replace symbols with values
    byte_pos = 0;
    for (u64 i = 0; i < ast->count; i++) {
        visit_NodeStatement2(&ast->statements[i], &old, &byte_pos);
    }
}