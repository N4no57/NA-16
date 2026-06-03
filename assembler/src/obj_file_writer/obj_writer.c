#include "obj_writer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

constexpr char zero = 0;

void string_table_append(ObjectFile *object_file, u64 *capacity, char *string) {
    if (object_file->header.string_table_size >= *capacity) {
        *capacity *= 2;
        char **tmp = realloc(object_file->string_table, *capacity * sizeof(char *));
        if (tmp == nullptr) {
            exit(1);
        }
        object_file->string_table = tmp;
    }

    object_file->string_table[object_file->header.string_table_size] = string;
    object_file->header.string_table_size++;
}

bool string_exists(ObjectFile *object_file, char *string) {
    if (!object_file->string_table) return false;

    for (u64 i = 0; i < object_file->header.string_table_size; i++) {
        if (strcmp(object_file->string_table[i], string) == 0) {
            return true;
        }
    }
    return false;
}

u64 get_string_ref(ObjectFile *object_file, char *string) {
    if (!object_file->string_table) return 0;

    u64 offset = 0;
    for (u64 i = 0; i < object_file->header.string_table_size; i++) {
        if (strcmp(object_file->string_table[i], string) == 0) {
            return offset;
        }
        offset += strlen(object_file->string_table[i]) + 1;
    }
    return -1;
}

void build_string_table(ObjectFile *object_file) {
    u64 string_table_capacity = 8;

    object_file->string_table = malloc(string_table_capacity * sizeof(char *));

    for (u64 i = 0; i < object_file->header.section_table_size; i++) {
        Section *section = &object_file->section_table[i];

        if (string_exists(object_file, section->name)) continue;

        string_table_append(object_file, &string_table_capacity, section->name);
    }

    for (u64 i = 0; i < object_file->header.symbol_table_size; i++) {
        NodeSymbol *symbol = &object_file->symbol_table[i];

        if (string_exists(object_file, symbol->symbol_name)) continue;

        string_table_append(object_file, &string_table_capacity, symbol->symbol_name);
    }
}

void write_obj(ObjectFile *object_file, char *filename) {
    build_string_table(object_file);

    FILE *f = fopen(filename, "wb");

    ObjectFileHeader *header = &object_file->header;

    // temporary header block
    fseek(f, sizeof(header->magic), SEEK_CUR);
    fseek(f, sizeof(header->version), SEEK_CUR);
    fseek(f, sizeof(header->section_table_offset), SEEK_CUR);
    fseek(f, sizeof(header->section_table_size), SEEK_CUR);
    fseek(f, sizeof(header->symbol_table_offset), SEEK_CUR);
    fseek(f, sizeof(header->symbol_table_size), SEEK_CUR);
    fseek(f, sizeof(header->relocation_table_offset), SEEK_CUR);
    fseek(f, sizeof(header->relocation_table_size), SEEK_CUR);
    fseek(f, sizeof(header->string_table_offset), SEEK_CUR);
    fseek(f, sizeof(header->string_table_size), SEEK_CUR);

    // section table
    header->section_table_offset = ftell(f);
    for (u64 i = 0; i < header->section_table_size; i++) {
        Section *section = &object_file->section_table[i];
        u64 str_table_ref = get_string_ref(object_file, section->name);
        fwrite(&str_table_ref, sizeof(str_table_ref), 1, f);
        fwrite(&section->offset, sizeof(section->offset), 1, f);
        fwrite(&section->count, sizeof(section->count), 1, f);
    }

    // symbol table
    header->symbol_table_offset = ftell(f);
    for (u64 i = 0; i < header->symbol_table_size; i++) {
        NodeSymbol *sym = &object_file->symbol_table[i];
        u64 str_table_ref = get_string_ref(object_file, sym->symbol_name);
        u8 status = zero << 1 | sym->global; // zero is placeholder for up and coming defined bool.
        fwrite(&str_table_ref, sizeof(str_table_ref), 1, f);
        fwrite(&sym->section_idx, sizeof(sym->section_idx), 1, f); // what section am I in?
        fwrite(&sym->value, sizeof(sym->value), 1, f); // where am I in the section
        fwrite(&status, sizeof(status), 1, f); // info about the symbol e.g. definition or if it is global
    }

    // relocation table
    header->relocation_table_offset = ftell(f);
    for (u64 i = 0; i < header->relocation_table_size; i++) {
        Relocation *reloc = &object_file->relocation_table[i];
        fwrite(&reloc->type, sizeof(reloc->type), 1, f);
        fwrite(&reloc->offset, sizeof(reloc->offset), 1, f);
    }

    // program data
    u64 program_offset = ftell(f);
    for (u64 i = 0; i < header->section_table_size; i++) {
        Section *section = &object_file->section_table[i];
        fwrite(section->data, section->count, 1, f);
    }

    // string table
    header->string_table_offset = ftell(f);
    for (u64 i = 0; i < header->string_table_size; i++) {
        u64 len = strlen(object_file->string_table[i]);
        fwrite(object_file->string_table[i], len+1, 1, f);
    }

    // fix offset for section table
    const _off64_t section_table_offset = (i64)header->section_table_offset;
    fseeko64(f, section_table_offset, SEEK_SET);
    for (u64 i = 0; i < header->section_table_size; i++) {
        fseek(f, sizeof(u64), SEEK_CUR);
        fwrite(&program_offset, sizeof(object_file->section_table[0].offset), 1, f);
        program_offset += object_file->section_table[0].count;
    }

    fseek(f, 0, SEEK_SET);
    fwrite(&header->magic, sizeof(header->magic), 1, f);
    fwrite(&header->version, sizeof(header->version), 1, f);
    fwrite(&header->section_table_offset, sizeof(header->section_table_offset), 1, f);
    fwrite(&header->section_table_size, sizeof(header->section_table_size), 1, f);
    fwrite(&header->symbol_table_offset, sizeof(header->symbol_table_offset), 1, f);
    fwrite(&header->symbol_table_size, sizeof(header->symbol_table_size), 1, f);
    fwrite(&header->relocation_table_offset, sizeof(header->relocation_table_offset), 1, f);
    fwrite(&header->relocation_table_size, sizeof(header->relocation_table_size), 1, f);
    fwrite(&header->string_table_offset, sizeof(header->string_table_offset), 1, f);
    fwrite(&header->string_table_size, sizeof(header->string_table_size), 1, f);

    fclose(f);
}
