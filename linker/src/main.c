#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/linklib.h"
#include "lib/global_symbol_table.h"
#include "lib/linked_section_table.h"
#include "lib/section_map.h"
#include "object_file_reader/obj_reader.h"


#define NUMBER_OF_OBJS 1 // TODO: make this multi object file because then wth is this even for?

i64 find_section(LinkedSectionTable *lst, u64 section_table_count, char *name) {
    for (int i = 0; i < section_table_count; i++) {
        if (strcmp(name, lst->sections[i].name) == 0) {
            return i;
        }
    }
    return -1;
}

void merge_sections(ObjectFile *objs, u64 obj_count) {
    SectionMapList sml = {0};
    sml_init(&sml, objs, obj_count);

    LinkedSectionTable lst = {0};
    lst_init(&lst);
    for (u64 i = 0; i < obj_count; i++) {
        ObjectFile *obj = &objs[i];
        for (u64 j = 0; j < obj->header.section_table_size; j++) {
            Section *sect = &obj->section_table[j];
            i64 sect_idx = find_section(&lst, lst.count, sect->name);
            if (sect_idx == -1) {
                LinkedSection new_sect = {0};
                new_sect.name = sect->name;

                lst_push(&lst, &new_sect);

                sect_idx = (i64)lst.current;
            }

            lst_merge(&lst, &sml, sect, sect_idx, i, j);
        }
    }
}

int main() {
    ObjectFile obj[1] = {0};
    char *filename = "test.o";
    obj->header.filename = filename;

    read_obj(obj, filename);


    // merge sections
    merge_sections(obj, 1);

    // assign section addresses
    u64 address_tracker = 0;
    for (u64 i = 0; i < sections_count; i++) {
        LinkedSection *section = &sections[i];
        section->address = address_tracker;
        address_tracker += section->size;
    }

    // build global symbol table
    GlobalSymbolTable table = {0};
    glt_init(&table);

    glt_push_table(&table, obj.symbol_table, obj.header.symbol_table_size, filename);

    // resolve symbol addresses
    for (u64 i = 0; i < table.count; i++) {
        for (u64 j = 0; j < table.items[i].count; j++) {
            Symbol *symbol = &table.items[i].symbols[j];
            SectionMap *map = &section_map[i][symbol->section_idx];

            LinkedSection *section = &sections[map->linked_section];

            symbol->address = section->address + map->offset_adjust + symbol->section_offset;
        }
    }

    // apply relocations
    for (u64 i = 0; i < NUMBER_OF_OBJS; i++) {
        for (u64 j = 0; j < obj.header.relocation_table_size; j++) {
            Relocation *relocation = &obj.relocation_table[j];
            SectionMap *map = &section_map[i][relocation->section_idx];

            LinkedSection *section = &sections[map->linked_section];
            Symbol *symbol = &table.items[i].symbols[relocation->symbol_ref];

            if (relocation->type == IMM_8) {
                section->data[relocation->section_offset+map->offset_adjust] = symbol->address;
            } else if (relocation->type == IMM_16) {
                section->data[relocation->section_offset+map->offset_adjust] = symbol->address & 0xFF;
                section->data[relocation->section_offset+map->offset_adjust+1] = symbol->address >> 8 & 0xFF;
            } else if (relocation->type == IMM_32) {
                section->data[relocation->section_offset+map->offset_adjust] = symbol->address & 0xFF;
                section->data[relocation->section_offset+map->offset_adjust+1] = symbol->address >> 8 & 0xFF;
                section->data[relocation->section_offset+map->offset_adjust+2] = symbol->address >> 16 & 0xFF;
                section->data[relocation->section_offset+map->offset_adjust+3] = symbol->address >> 24 & 0xFF;
            } else if (relocation->type == REL_8) {
                // Disclaimer: The relative relocations come with the ISA specifics of relatives only being used by conditional jumps and therefore being the only operand
                i64 value = (i64)relocation->section_offset;
                value += (i64)map->offset_adjust + 1;
                value += (i64)section->address;
                value = (i64)symbol->address - value;
                section->data[relocation->section_offset + map->offset_adjust] = value;
            } else if (relocation->type == REL_16) {
                i64 value = (i64)relocation->section_offset;
                value += (i64)map->offset_adjust + 2;
                value += (i64)section->address;
                value = (i64)symbol->address - value;
                section->data[relocation->section_offset+map->offset_adjust] = value & 0xFF;
                section->data[relocation->section_offset+map->offset_adjust+1] = value >> 8 & 0xFF;
            } else if (relocation->type == REL_32) {
                i64 value = (i64)relocation->section_offset;
                value += (i64)map->offset_adjust + 4;
                value += (i64)section->address;
                value = (i64)symbol->address - value;
                section->data[relocation->section_offset+map->offset_adjust] = value & 0xFF;
                section->data[relocation->section_offset+map->offset_adjust+1] = value >> 8 & 0xFF;
                section->data[relocation->section_offset+map->offset_adjust+2] = value >> 16 & 0xFF;
                section->data[relocation->section_offset+map->offset_adjust+3] = value >> 24 & 0xFF;
            }
        }
    }

    FILE *f = fopen("test.bin", "wb");
    for (u64 i = 0; i < sections_count; i++) {
        fwrite(sections[i].data, sections[i].size, 1, f);
    }
    fclose(f);

    free(obj.section_table[0].data);
    free(obj.section_table);
    free(obj.symbol_table);
    free(obj.relocation_table);
    free(obj.string_table);
    free(table.items);
    free(sections);

    for (u64 i = 0; i < NUMBER_OF_OBJS; i++) {
        free(section_map[i]);
    }
    free(section_map);

    return 0;
}