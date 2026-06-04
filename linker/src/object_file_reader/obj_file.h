#ifndef NA_16_OBJ_FILE_H
#define NA_16_OBJ_FILE_H

#include "../lib/linklib.h"

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

    u8 *data; // program data that the section table indexes to
} ObjectFile;

#endif //NA_16_OBJ_FILE_H
