#include <stdlib.h>

#include "section_map.h"

SectionMapList *sml_init(SectionMapList *sml, ObjectFile *objs, u64 obj_count) {
    sml->obj_count = obj_count;

    sml->section_map = malloc(sizeof(SectionMap *) * obj_count);

    for (u64 i = 0; i < sml->obj_count; i++) {
        sml->section_map[i] = malloc(sizeof(SectionMap) * objs[i].header.section_table_size);
    }
    return sml;
}

void sml_free(SectionMapList *sml) {
    for (u64 i = 0; i < sml->obj_count; i++) {
        free(sml->section_map[i]);
    }

    free(sml->section_map);
}
