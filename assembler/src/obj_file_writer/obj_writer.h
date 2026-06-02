#ifndef NA_16_OBJ_WRITER_H
#define NA_16_OBJ_WRITER_H

#include "../lib/asmlib.h"
#include "sections.h"
#include "../parser/ast.h"

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
    ObjectFileHeader header;
    Section *section_table;
    NodeSymbol *symbol_table;
    // Relocation *relocation_table;
    char **string_table;
} ObjectFile;

void write_obj(ObjectFile *object_file, char *filename);

#endif //NA_16_OBJ_WRITER_H
