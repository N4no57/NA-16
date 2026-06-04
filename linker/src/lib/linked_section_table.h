#ifndef NA_16_LINKED_SECTION_TABLE_H
#define NA_16_LINKED_SECTION_TABLE_H

#include "linklib.h"
#include "section_map.h"
#include "../object_file_reader/obj_file.h"

typedef struct {
    char *name;

    u64 address;
    u64 size;

    u8 *data;
} LinkedSection;

typedef struct {
    LinkedSection *sections;
    u64 count;
    u64 size;
    u64 current;
} LinkedSectionTable;

LinkedSectionTable *lst_init(LinkedSectionTable *table);

void lst_free(LinkedSectionTable *table);

void lst_push(LinkedSectionTable *table, LinkedSection *section);

void lst_merge(LinkedSectionTable *table, SectionMapList *sml, Section *section, u64 linked_sect_idx, u64 obj_idx, u64 sect_idx) ;

#endif //NA_16_LINKED_SECTION_TABLE_H
