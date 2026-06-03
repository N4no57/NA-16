#ifndef NA_16_RELOCATIONS_H
#define NA_16_RELOCATIONS_H

#include "../lib/asmlib.h"

typedef enum {
    IMM_8,
    IMM_16,
    IMM_32,

    REL_8,
    REL_16,
    REL_32,
} RelocationType;

typedef struct {
    char *name;
    u64 offset;
    u64 section_idx;
    u64 symbol_ref;
    RelocationType type;
} Relocation;

typedef struct {
    Relocation *relocations;
    u64 count;
    u64 size;
} RelocationTable;

void init_RelocationTable(RelocationTable *list);
void free_RelocationTable(RelocationTable *list);

void relocation_push(RelocationTable *list, Relocation *relocation);
Relocation *get_relocation(RelocationTable *list, char *name);

#endif //NA_16_RELOCATIONS_H
