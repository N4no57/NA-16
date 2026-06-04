#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/linklib.h"
#include "object_file_reader/obj_reader.h"

typedef struct {
    struct {
        char *filename;
        Symbol *symbols;
        u64 count;
    } *items;

    struct {
        Symbol *symbols;
        u64 count;
    } global_symbols;

    u64 count;
    u64 size;
} GlobalSymbolTable;

typedef struct {
    u64 linked_section; // what section in the global section table is it?
    u64 offset_adjust; // where is the original section's data in the linked section?
} SectionMap;

typedef struct {
    char *name;

    u64 address;
    u64 size;

    u8 *data;
} LinkedSection;

#define NUMBER_OF_OBJS 1 // TODO: make this multi object file because then wth is this even for?

i64 find_section(LinkedSection *sections, u64 section_table_count, char *name) {
    for (int i = 0; i < section_table_count; i++) {
        if (strcmp(name, sections[i].name) == 0) {
            return i;
        }
    }
    return -1;
}

int main() {
    ObjectFile obj;
    char *filename = "test.o";
    obj.header.filename = filename;

    read_obj(&obj, filename);


    // merge sections
    SectionMap **section_map = malloc(sizeof(SectionMap *) * NUMBER_OF_OBJS);
    for (u64 i = 0; i < NUMBER_OF_OBJS; i++) {
        section_map[i] = malloc(sizeof(SectionMap) * obj.header.section_table_size);
    }

    u64 sections_count = 0;
    u64 sections_size = 8;
    LinkedSection *sections = malloc(sections_size * sizeof(LinkedSection));
    for (u64 i = 0; i < NUMBER_OF_OBJS; i++) {
        for (u64 j = 0; j < obj.header.section_table_size; j++) {
            Section *sect = &obj.section_table[j];
            i64 sect_idx =  find_section(sections, sections_count, sect->name);
            if (sect_idx == -1) {
                if (sections_count >= sections_size) {
                    if (sections_size == 0) sections_size = 8;
                    else sections_size *= 2;
                    LinkedSection *tmp = realloc(sections, sections_size * sizeof(LinkedSection));
                    if (!tmp) {
                        fprintf(stderr, "realloc failed\nget more RAM broke boy");
                        exit(1);
                    }
                    sections = tmp;
                }

                LinkedSection new_sect = {0};
                new_sect.address = 0;
                new_sect.size = 0;
                new_sect.name = sect->name;
                new_sect.data = nullptr;
                sections[sections_count] = new_sect;
                sect_idx = (i32)sections_count;
                sections_count++;
            }

            LinkedSection *linked_section = &sections[sect_idx];
            section_map[i][j].linked_section = sect_idx;
            section_map[i][j].offset_adjust = linked_section->size;
            linked_section->size += sect->size;
            linked_section->data = realloc(sections[sect_idx].data, linked_section->size * sizeof(u8));
            memcpy(&linked_section->data[section_map[i][j].offset_adjust], sect->data, sect->size * sizeof(u8));
        }
    }

    // assign section addresses
    u64 address_tracker = 0;
    for (u64 i = 0; i < sections_count; i++) {
        LinkedSection *section = &sections[i];
        section->address = address_tracker;
        address_tracker += section->size;
    }

    // build global symbol table
    GlobalSymbolTable table = {0};
    table.count = 1;
    table.size = 8;
    table.items = malloc(table.size * sizeof(table.items[0]));

    table.items[0].filename = filename;
    table.items[0].symbols = obj.symbol_table;
    table.items[0].count = obj.header.symbol_table_size;

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