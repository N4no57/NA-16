#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef struct {
    u32 magic;
    u16 version;

    u64 section_table_offset; // file offset
    u64 section_table_size; // how many sections?

    u64 symbol_table_offset; // file offset
    u64 symbol_table_size; // how many symbols?

    u64 relocation_table_offset; // file offset
    u64 relocation_table_size; // how many relocations?

    u64 string_table_offset; // file offset
    u64 string_table_size; // how many strings?

    // bookeeping
    char *filename;
} ObjectFileHeader;

typedef struct {
    char *name;
    u64 offset;
    u64 size;

    u8 *data;
    u64 str_table_ref;
} Section;

typedef enum {
    SYM_GLOBAL = 1 << 0,
    SYM_DEFINED = 1 << 1,
} SymbolFlags;

typedef struct {
    char *name;
    u64 section_idx;
    u64 section_offset;

    u8 flags;

    u64 str_table_ref;
    u64 address; // true address
} Symbol;

typedef enum {
    IMM_8,
    IMM_16,
    IMM_32,

    REL_8,
    REL_16,
    REL_32
} RelocationType;

typedef struct {
    char *name;
    u64 symbol_ref;
    RelocationType type;
    u64 section_offset;
    u64 section_idx;
} Relocation;

typedef struct {
    ObjectFileHeader header;
    Section *section_table;
    Symbol *symbol_table;
    Relocation *relocation_table;
    char *string_table;
} ObjectFile;

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