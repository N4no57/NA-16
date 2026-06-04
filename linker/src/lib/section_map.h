#ifndef NA_16_SECTION_MAP_H
#define NA_16_SECTION_MAP_H

#include "linklib.h"
#include "../object_file_reader/obj_file.h"

typedef struct {
    u64 linked_section; // what section in the global section table is it?
    u64 offset_adjust; // where is the original section's data in the linked section?
} SectionMap;

typedef struct {
    SectionMap **section_map;
    u64 count;
    u64 size;
} SectionMapList;

SectionMapList *sml_init(SectionMapList *sml, ObjectFile *objs, u64 obj_count);

#endif //NA_16_SECTION_MAP_H
