#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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
} ObjectFileHeader;

typedef struct {
    char *name;
    u64 offset;
    u64 size;
} Section;

typedef struct {
    char *name;
    u64 section_idx;
    u64 section_offset;
    u8 status;
} Symbol;

typedef enum {
    IMM_8,
    IMM_16,
    REL_8,
    REL_16
} RelocationType;

typedef struct {
    char *name;
    u64 symbol_ref;
    RelocationType type;
    u64 section_offset;
} Relocation;

typedef struct {
    ObjectFileHeader header;
    Section *section_table;
    Symbol *symbol_table;
    Relocation *relocation_table;
    char **string_table;
} ObjectFile;

int main() {
    ObjectFile obj;
    ObjectFileHeader *header = &obj.header;

    FILE *f = fopen("test.o", "rb");
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

    long thing = ftell(f);
    fseek(f, 0, SEEK_END);
    long thing_2 = ftell(f);
    long size = thing_2 - thing;

    void *ptr = malloc(size);

    fseek(f, thing, SEEK_SET);
    fread(ptr, size, 1, f);

    fclose(f);
    free(ptr);

    return 0;
}