#ifndef NA_16_SECTIONS_H
#define NA_16_SECTIONS_H

#include "asmlib.h"

typedef struct {
    char *name;

    // binary data for section
    u8 *data;
    u64 count;
    u64 size;
} Section;

typedef struct {
    Section *sections;
    u64 count;
    u64 size;

    u64 current;
} SectionTable;

void init_SectionTable(SectionTable *list);
void free_SectionTable(SectionTable *list);

void section_push(SectionTable *list, Section *section);
Section *get_section(SectionTable *list, char *name);

#endif //NA_16_SECTIONS_H
