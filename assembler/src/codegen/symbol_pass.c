#include <stdlib.h>
#include <string.h>

#include "codegen.h"
#include "../lib/error.h"
#include "../obj_file_writer/sections.h"

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
        NodeSymbol *new_sym = find_symbol(new, old->symbols[i].symbol_name); // get matching symbol
        if (old->symbols[i].kind == SK_LABEL && new_sym->kind == SK_LABEL) {
            ret_val =  old->symbols[i].value == new_sym->value;
        }
    }
    return ret_val;
}

bool wont_fit_u8(u64 value)
{
    return value > UINT8_MAX;   // 255
}

bool wont_fit_s8(i64 value)
{
    return value < INT8_MIN || value > INT8_MAX;   // -128 to 127
}

bool require_16_bits(const NodeInstruction *node, const InstructionSignature *sig) {
    bool use_16bits = false;
    if (node->operand_size == 0) {
        for (i32 i = 0; i < sig->operand_count; i++) {
            if (sig->kinds[i] == IMMEDIATE) {
                use_16bits = wont_fit_u8(node->operands[i].immediate);
            } else if (sig->kinds[i] == DISPLACEMENT) {
                use_16bits = wont_fit_s8(node->operands[i].immediate);
            }
            if (use_16bits) break;
        }
    } else {
        if (node->operand_size == 2) {
            use_16bits = true;
        }
    }
    return use_16bits;
}

void handle_globals(NodeDirective *node, SymbolTable *table) {
    u64 tok_idx = 0;
    Token *tok = &node->args.tokens[tok_idx];

    if (strcmp(node->name, ".global") == 0) {
        while (tok_idx < node->args.count) {
            if (tok->type != TT_IDENTIFIER) {
                error(tok->pos, "Invalid argument for \"%s\" directive", node->name);
            }

            NodeSymbol *symbol = find_symbol(table, tok->value);

            if (symbol == nullptr) {
                error(tok->pos, "Undefined symbol reference \"%s\"", tok->value);
            }

            symbol->global = true;

            tok = &node->args.tokens[++tok_idx];
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Symbol table generation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void capture_symbol(NodeSymbol *node, SymbolTable *table, SectionTable *sections) {
    NodeSymbol *symbol = find_symbol(table, node->symbol_name);

    if (symbol) {
        error(node->pos, "Reused symbol");
    }

    if (node->kind == SK_LABEL) { // it's a label
        node->value = 0;
    }

    node->section_idx = sections->current;

    push_symbol(table, *node);
}

void capture_segment(NodeDirective *node, SectionTable *sections) {
    if (strcmp(node->name, ".section") == 0) {
        if (node->args.count > 1) error(node->pos, "Excuse me what the actual fu-");

        if (get_section(sections, node->args.tokens[0].value)) return; // if the section already exists then do nothing

        Section s = {node->args.tokens[0].value, nullptr, 0, 0};

        section_push(sections, &s);

        sections->current = sections->count - 1; // set new section as current
    }
}

void visit_NodeStatement1(NodeStatement *node, SymbolTable *table, SectionTable *sections) {
    if (node->kind == ST_INSTRUCTION) {
        return;
    }

    if (node->kind == ST_DIRECTIVE) {
        capture_segment(&node->directive, sections);
        return;
    }

    if (node->kind == ST_SYMBOL) {
        capture_symbol(&node->symbol, table, sections);
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

        if (symbol == nullptr) { // undefined symbol reference; oooh scary
            fatal(operand->pos, "Undefined symbol reference \"%s\"", operand->symbol_name);
        }

        u8 inc_by = 0;
        if (!use_16bits) {
            bool need_aex = false;
            need_aex = wont_fit_u8(symbol->value);
            inc_by += need_aex == true ? 2 : 0; // adding 2 accounts for new prefix and extra byte for the symbol
        } else {
            inc_by++;
        }

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
        return;
    }

    error(operand->pos, "Invalid operand type");
}

#define MEX_PREFIX_SIZE 2
#define BASE_ENCODING_SIZE 2
#define SHORT_DISP_SIZE 1
#define LONG_DISP_SIZE 2
#define AEX_SIZE 1

u64 visit_NodeInstructionRecalc(const NodeInstruction *node, const SymbolTable *table, SectionTable *sections) {
    u64 ret_val = 0;

    if (is_cond_jump(node->mnemonic)) {
        // handle this crap
        NodeSymbol *symbol = find_symbol(table, node->operands[0].symbol_name);
        u64 size = BASE_ENCODING_SIZE;

        i32 delta = (i32)symbol->value - (sections->sections[sections->current].count + BASE_ENCODING_SIZE + SHORT_DISP_SIZE);

        if (wont_fit_s8(delta)) {
            size += AEX_SIZE + LONG_DISP_SIZE; // increase by 3 to account for AEX byte and 2 bytes of displacement
        } else {
            size += SHORT_DISP_SIZE;
        }

        return size;
    }

    InstructionSpec info = get_spec(node->mnemonic);

    u64 sig_id = 0;
    InstructionSignature *sig = &info.signatures[sig_id];
    for (sig_id = 0; sig_id < info.signature_count; sig_id++) {
        if (match_signature(node, sig)) break;
        sig = &info.signatures[sig_id+1];
    }

    if (sig_id > 0) {
        ret_val += MEX_PREFIX_SIZE;
    }

    bool use_16bits = require_16_bits(node, sig);

    if (use_16bits) {
        ret_val++; // AEX prefix is 1 byte
    }

    if (info.opcode > 0xF) {
        ret_val++; // escape byte is... well... 1 byte
    }

    ret_val += BASE_ENCODING_SIZE; // instruction encoding

    for (int i = 0; i < node->operand_count; i++) {
        visit_NodeOperandRecalc(&node->operands[i], table, use_16bits, &ret_val);
    }

    return ret_val;
}

u64 handle_define(const NodeDirective *node, u64 size) {
    u64 ret_val = 0;

    u64 tok_idx = 0;
    Token *tok = &node->args.tokens[tok_idx];

    while (tok_idx < node->args.count) {
        if (tok->type != TT_IMMEDIATE) {
            error(tok->pos, "invalid argument for \"%s\" directive", node->name);
        }

        ret_val += size;

        tok = &node->args.tokens[++tok_idx];
    }

    return ret_val;
}

u64 visit_NodeDirectiveRecalc(const NodeDirective *node, SectionTable *sections) {
    if (strcmp(node->name, ".global") == 0) return 0; // early return as this has already been handled

    if (strcmp(node->name, ".section") == 0) {
        Section *sect = get_section(sections, node->args.tokens[0].value);

        u64 tmp1 = (u64)sect;
        u64 tmp2 = (u64)&sections->sections[0];
        u64 sect_idx = tmp2 - tmp1;

        sections->current = sect_idx;
        return 0;
    }

    u64 ret_val = 0;
    u64 tok_idx = 0;
    Token *tok = &node->args.tokens[tok_idx];

    if (strcmp(node->name, ".db") == 0) {
        return handle_define(node, 1);
    }

    if (strcmp(node->name, ".dw") == 0) {
        return handle_define(node, 2);
    }

    if (strcmp(node->name, ".dd") == 0) {
        return handle_define(node, 4);
    }

    if (strcmp(node->name, ".dq") == 0) {
        return handle_define(node, 8);
    }

    if (strcmp(node->name, ".ascii") == 0) {
        while (tok_idx < node->args.count) {
            if (tok->type != TT_STRING) {
                error(tok->pos, "invalid argument for \"%s\" directive", node->name);
            }

            String *s = tok->value;

            ret_val += s->size;

            tok = &node->args.tokens[++tok_idx];
        }

        return ret_val;
    }

    if (strcmp(node->name, ".asciz") == 0) {
        while (tok_idx < node->args.count) {
            if (tok->type != TT_STRING) {
                error(tok->pos, "invalid argument for \"%s\" directive", node->name);
            }

            String *s = tok->value;

            ret_val += s->size + 1; // account for added null terminator at the end

            tok = &node->args.tokens[++tok_idx];
        }

        return ret_val;
    }

    error(node->pos, "invalid directive");

    return 0;
}

u64 visit_NodeStatementRecalc(NodeStatement *node, const SymbolTable *table, SectionTable *sections) {
    u64 ret_val = 0;
    if (node->kind == ST_INSTRUCTION) {
        ret_val = visit_NodeInstructionRecalc(&node->instruction, table, sections);
    } else if (node->kind == ST_DIRECTIVE) {
        ret_val = visit_NodeDirectiveRecalc(&node->directive, sections);
    }
    return ret_val;
}

void recalc_layout(const NodeProgram *ast, const SymbolTable *old, SymbolTable *new, SectionTable *sections) {
    new->count = old->count;
    new->size = old->size;
    new->symbols = malloc(new->size * sizeof(NodeSymbol));
    memcpy(new->symbols, old->symbols, old->size * sizeof(NodeSymbol));

    u64 *sizes = malloc(ast->count * sizeof(u64));

    for (u64 i = 0; i < ast->count; i++) {
        sizes[i] = visit_NodeStatementRecalc(&ast->statements[i], new, sections);
        sections->sections[sections->current].count += sizes[i];
    }

    for (u64 i = 0; i < sections->count; i++) {
        sections->sections[i].count = 0;
    }

    for (u64 i = 0; i < ast->count; i++) {
        if (ast->statements[i].kind == ST_SYMBOL && ast->statements[i].symbol.kind == SK_LABEL) {
            NodeSymbol *symbol = find_symbol(new, ast->statements[i].symbol.symbol_name);
            symbol->value = (i32)sections->sections[sections->current].count;
        } else if (ast->statements[i].kind == ST_DIRECTIVE) {
            if (strcmp(ast->statements[i].directive.name, ".section") == 0) {
                Section *sect = get_section(sections, ast->statements[i].directive.args.tokens[0].value);

                u64 tmp1 = (u64)sect;
                u64 tmp2 = (u64)&sections->sections[0];
                u64 sect_idx = tmp2 - tmp1;

                sections->current = sect_idx;
            }
        }

        sections->sections[sections->current].count += sizes[i];
    }

    for (u64 i = 0; i < sections->count; i++) {
        sections->sections[i].count = 0;
    }

    free(sizes);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Symbol table usage
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void visit_NodeOperand2(const char *mnemonic, NodeOperand *operand, SymbolTable *table, const u64 *byte_pos) {
    if (operand->kind == REGISTER || operand->kind == REG_INDIRECT) {
        return;
    }

    if (operand->kind == SYMBOL) {
        NodeSymbol *symbol = find_symbol(table, operand->symbol_name);

        operand->kind = IMMEDIATE;
        operand->immediate = symbol->value;

        if (is_cond_jump(mnemonic)) {
            operand->kind = DISPLACEMENT;
            if (symbol->kind == SK_LABEL) {
                operand->immediate = symbol->value - *(i64 *)byte_pos;
            }
        }

        return;
    }

    if (operand->kind == IMMEDIATE || operand->kind == DISPLACEMENT) {
        return;
    }

    error(operand->pos, "Invalid operand type");
}

void visit_NodeInstruction2(NodeInstruction *node, SymbolTable *table, const u64 *byte_pos) {
    for (int i = 0; i < node->operand_count; i++) {
        visit_NodeOperand2(node->mnemonic, &node->operands[i], table, byte_pos);
    }
}

void visit_NodeStatement2(NodeStatement *node, SymbolTable *table, const u64 *byte_pos) {
    if (node->kind == ST_INSTRUCTION) {
        visit_NodeInstruction2(&node->instruction, table, byte_pos);
    }
}

void patch_symbols(const NodeProgram *ast, SymbolTable *table, SectionTable *sections) {
    SymbolTable tmp = {0};
    tmp.count = table->count;
    tmp.size = table->size;
    tmp.symbols = malloc(tmp.size * sizeof(NodeSymbol));
    memcpy(tmp.symbols, table->symbols, table->size * sizeof(NodeSymbol));

    u64 *sizes = malloc(ast->count * sizeof(u64));

    u16 simulated_pc = 0;
    for (u64 i = 0; i < ast->count; i++) {
        sizes[i] = visit_NodeStatementRecalc(&ast->statements[i], &tmp, sections);
        sections->sections[sections->current].count += sizes[i];
    }
    free(tmp.symbols);

    for (u64 i = 0; i < sections->count; i++) {
        sections->sections[i].count = 0;
    }

    u64 byte_pos = 0;
    for (u64 i = 0; i < ast->count; i++) {
        sections->sections[sections->current].count += sizes[i]; // set byte pos to the end of the instruction in which we are visiting
        byte_pos = 0;
        for (u64 j = 0; j < sections->current+1; j++) {
            byte_pos += sections->sections[j].count;
        }
        visit_NodeStatement2(&ast->statements[i], table, &byte_pos);
    }

    for (u64 i = 0; i < sections->count; i++) {
        sections->sections[i].count = 0;
    }

    free(sizes);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Symbol Pass
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void symbol_pass(NodeProgram *ast, SymbolTable *symbols, SectionTable *sections, RelocationTable *relocationTable) {
    if (!ast) return;

    SymbolTable old = {0}, new = {0};
    SymbolTable *old_ptr = &old, *new_ptr = &new;

    // generate symbol table
    for (u64 i = 0; i < ast->count; i++) {
        visit_NodeStatement1(&ast->statements[i], old_ptr, sections);
    }

    // check for globals
    for (u64 i = 0; i < ast->count; i++) {
        if (ast->statements[i].kind == ST_DIRECTIVE) {
            handle_globals(&ast->statements[i].directive, old_ptr);
        }
    }

    halt_on_error();

    // recalculate and correct symbols
    do {
        recalc_layout(ast, old_ptr, new_ptr, sections);
        halt_on_error();
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
    patch_symbols(ast, old_ptr, sections);

    memcpy(symbols, old_ptr, sizeof(SymbolTable));
}