#include <stdio.h>

#include "obj_reader.h"

#include <stdlib.h>

void read_obj(ObjectFile *obj, char *filename) {
    ObjectFileHeader *header = &obj->header;

    FILE *f = fopen(filename, "rb");
    fread(&header->magic, sizeof(header->magic), 1, f);
    fread(&header->version, sizeof(header->version), 1, f);
    fread(&header->section_table_offset, sizeof(header->section_table_offset), 1, f);
    fread(&header->section_table_size, sizeof(header->section_table_size), 1, f);
    fread(&header->symbol_table_offset, sizeof(header->symbol_table_offset), 1, f);
    fread(&header->symbol_table_size, sizeof(header->symbol_table_size), 1, f);
    fread(&header->relocation_table_offset, sizeof(header->relocation_table_offset), 1, f);
    fread(&header->relocation_table_size, sizeof(header->relocation_table_size), 1, f);
    fread(&header->string_table_offset, sizeof(header->string_table_offset), 1, f);
    fread(&header->string_table_size, sizeof(header->string_table_size), 1, f);

    long eoh = ftell(f);

    fseek(f, 0, SEEK_END);
    u64 string_table_end = ftell(f);
    obj->string_table = malloc(string_table_end - header->string_table_offset);
    fseek(f, (long)header->string_table_offset, SEEK_SET);
    fread(obj->string_table, string_table_end - header->string_table_offset, 1, f);

    fseek(f, eoh, SEEK_SET);

    // section table
    obj->section_table = malloc(header->section_table_size * sizeof(Section));
    for (u64 i = 0; i < header->section_table_size; i++) {
        Section *section = &obj->section_table[i];
        fread(&section->str_table_ref, sizeof(section->str_table_ref), 1, f);
        fread(&section->offset, sizeof(section->offset), 1, f);
        fread(&section->size, sizeof(section->size), 1, f);
        section->name = obj->string_table + section->str_table_ref;
    }

    // symbol table
    obj->symbol_table = malloc(header->symbol_table_size * sizeof(Symbol));
    for (u64 i = 0; i < header->symbol_table_size; i++) {
        Symbol *sym = &obj->symbol_table[i];
        fread(&sym->str_table_ref, sizeof(sym->str_table_ref), 1, f);
        fread(&sym->section_idx, sizeof(sym->section_idx), 1, f); // what section am I in?
        fread(&sym->section_offset, sizeof(sym->section_offset), 1, f); // where am I in the section
        fread(&sym->flags, sizeof(sym->flags), 1, f); // info about the symbol e.g. definition or if it is global
        sym->name = obj->string_table + sym->str_table_ref;
    }

    // relocation table
    obj->relocation_table = malloc(header->relocation_table_size * sizeof(Relocation));
    for (u64 i = 0; i < header->relocation_table_size; i++) {
        Relocation *reloc = &obj->relocation_table[i];
        fread(&reloc->symbol_ref, sizeof(reloc->symbol_ref), 1, f);
        fread(&reloc->type, sizeof(reloc->type), 1, f);
        fread(&reloc->section_offset, sizeof(reloc->section_offset), 1, f);
        fread(&reloc->section_idx, sizeof(reloc->section_idx), 1, f);
        reloc->name = obj->symbol_table[reloc->symbol_ref].name;
    }

    u64 program_offset = ftell(f);
    u64 program_size = header->string_table_offset - program_offset;
    void *program = malloc(program_size);
    fread(program, program_size, 1, f);

    fclose(f);

    u64 offset = 0;
    for (u64 i = 0; i < obj->header.section_table_size; i++) {
        Section *section = &obj->section_table[i];

        if (section->size == 0) {
            section->data = nullptr;
            continue;
        }

        section->data = program + offset;
        offset += section->size;
    }
}
