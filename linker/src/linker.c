#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "lib/global_symbol_table.h"
#include "lib/linked_section_table.h"
#include "lib/section_map.h"
#include "linker.h"

i64 find_section(LinkedSectionTable *lst, char *name) {
    for (int i = 0; i < lst->count; i++) {
        if (strcmp(name, lst->sections[i].name) == 0) {
            return i;
        }
    }
    return -1;
}

void merge_sections(ObjectFile *objs, u64 obj_count, SectionMapList *sml, LinkedSectionTable *lst) {
    sml_init(sml, objs, obj_count);
    lst_init(lst);

    for (u64 i = 0; i < obj_count; i++) {
        ObjectFile *obj = &objs[i];
        for (u64 j = 0; j < obj->header.section_table_size; j++) {
            Section *sect = &obj->section_table[j];
            i64 sect_idx = find_section(lst, sect->name);
            if (sect_idx == -1) {
                LinkedSection new_sect = {0};
                new_sect.name = sect->name;

                lst_push(lst, &new_sect);

                sect_idx = (i64)lst->current;
            }

            lst_merge(lst, sml, sect, sect_idx, i, j);
        }
    }
}

void assign_section_addresses(LinkedSectionTable *lst) {
    u64 address_tracker = 0;
    for (u64 i = 0; i < lst->count; i++) {
        LinkedSection *section = &lst->sections[i];
        section->address = address_tracker;
        address_tracker += section->size;
    }
}

void resolve_symbol_addresses(const GlobalSymbolTable *glt, const SectionMapList *sml, const LinkedSectionTable *lst) {
    for (u64 i = 0; i < glt->count; i++) {
        for (u64 j = 0; j < glt->items[i].count; j++) {
            Symbol *symbol = &glt->items[i].symbols[j];

            if ((symbol->flags & SYM_DEFINED) != SYM_DEFINED) continue; // skip all undefined ones because yes

            const SectionMap *map = &sml->section_map[i][symbol->section_idx];

            const LinkedSection *section = &lst->sections[map->linked_section];

            symbol->address = section->address + map->offset_adjust + symbol->section_offset;
        }
    }

    for (u64 i = 0; i < glt->global_symbols.count; i++) {
        Symbol *symbol = &glt->global_symbols.symbols[i];
        u64 file_ref = glt->global_symbols.file_refs[i];
        const SectionMap *map = &sml->section_map[file_ref][symbol->section_idx]; // how to check what file its from?

        const LinkedSection *section = &lst->sections[map->linked_section];

        symbol->address = section->address + map->offset_adjust + symbol->section_offset;
    }
}

bool is_imm_reloc(const RelocationType type) {
    switch (type) {
        case IMM_8:
        case IMM_16:
        case IMM_32:
            return true;
        default:
            return false;
    }
}

u8 get_reloc_size(const RelocationType type) {
    if (type == IMM_8) return 1;
    if (type == IMM_16) return 2;
    if (type == IMM_32) return 4;

    if (type == REL_8) return 1;
    if (type == REL_16) return 2;
    if (type == REL_32) return 4;

    return 0;
}

void reloc_imm(Relocation *relocation, SectionMap *map, LinkedSection *section, Symbol *symbol) {
    u8 size = get_reloc_size(relocation->type);

    for (u8 i = 0; i < size; i++) {
        section->data[relocation->section_offset+map->offset_adjust+i] = symbol->address >> (i * 8) & 0xFF;
    }
}

void reloc_rel(Relocation *relocation, SectionMap *map, LinkedSection *section, Symbol *symbol) {
    // Disclaimer: The relative relocations come with the ISA specifics of relatives only being used by conditional jumps and therefore being the only operand
    const u8 size = get_reloc_size(relocation->type);

    i64 value = (i64)relocation->section_offset;
    value += (i64)map->offset_adjust + size;
    value += (i64)section->address;
    value = (i64)symbol->address - value;

    for (u8 i = 0; i < size; i++) {
        section->data[relocation->section_offset + map->offset_adjust+i] = value >> (i * 8) & 0xFF;
    }
}

void link(ObjectFile *objs, u64 obj_count, char *outfile) {
    // merge sections
    LinkedSectionTable lst = {0};
    SectionMapList sml = {0};
    merge_sections(objs, obj_count, &sml, &lst);

    // assign section addresses
    assign_section_addresses(&lst);

    // build global symbol table
    GlobalSymbolTable glt = {0};
    glt_init(&glt);

    for (u64 i = 0; i < obj_count; i++) {
        glt_push_table(&glt, objs[i].symbol_table, objs[i].header.symbol_table_size, objs[i].header.filename);
    }

    // resolve symbol addresses
    resolve_symbol_addresses(&glt, &sml, &lst);

    // apply relocations
    for (u64 i = 0; i < obj_count; i++) {
        for (u64 j = 0; j < objs[i].header.relocation_table_size; j++) {
            Relocation *relocation = &objs[i].relocation_table[j];
            SectionMap *map = &sml.section_map[i][relocation->section_idx];

            LinkedSection *section = &lst.sections[map->linked_section];
            Symbol *symbol = &glt.items[i].symbols[relocation->symbol_ref];

            if ((symbol->flags & SYM_DEFINED) != SYM_DEFINED) {
                // check the global symbols
                symbol = glt_get_global(&glt, symbol->name);

                if (!symbol) {
                    printf("the big error");
                    exit(-10);
                }
            }

            if (is_imm_reloc(relocation->type)) {
                reloc_imm(relocation, map, section, symbol);
            } else {
                reloc_rel(relocation, map, section, symbol);
            }
        }
    }

    FILE *f = fopen(outfile, "wb");
    for (u64 i = 0; i < lst.count; i++) {
        fwrite(lst.sections[i].data, lst.sections[i].size, 1, f);
    }
    fclose(f);

    glt_free(&glt);
    lst_free(&lst);
    sml_free(&sml);
}