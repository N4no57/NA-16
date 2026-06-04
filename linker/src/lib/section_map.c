#include <stdlib.h>

#include "section_map.h"

SectionMapList *sml_init(SectionMapList *sml, ObjectFile *objs, u64 obj_count) {
    sml->section_map = malloc(sizeof(SectionMap *) * obj_count);
    for (u64 i = 0; i < obj_count; i++) {
        sml->section_map[i] = malloc(sizeof(SectionMap) * objs[i].header.section_table_size);
    }
    return sml;
}
