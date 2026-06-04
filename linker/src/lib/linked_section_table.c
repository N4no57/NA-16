#include <stdlib.h>
#include <string.h>

#include "linked_section_table.h"
#include "section_map.h"
#include "../object_file_reader/obj_file.h"

LinkedSection defaultSections[] = {
    {"text", 0, 0, nullptr},
    {"data", 0, 0, nullptr},
    {"bss", 0, 0, nullptr},
    {"rodata", 0, 0, nullptr},
};

LinkedSectionTable *lst_init(LinkedSectionTable *table) {
    table->count = 4;
    table->size = 8;
    table->sections = malloc(sizeof(LinkedSection) * table->size);

    for (u64 i = 0; i < sizeof(defaultSections) / sizeof(defaultSections[0]); i++) {
        table->sections[i] = defaultSections[i];
    }

    return table;
}

void lst_free(LinkedSectionTable *table) {
    free(table->sections);
}

void lst_push(LinkedSectionTable *table, LinkedSection *section) {
    if (table->count >= table->size) {
        table->size *= 2;
        LinkedSection *tmp = realloc(table->sections, table->size * sizeof(*table->sections));
        if (tmp == nullptr) {
            exit(-1);
        }
        table->sections = tmp;
    }

    table->sections[table->count] = *section;
    table->current = table->count;

    table->count++;
}

void lst_merge(
    LinkedSectionTable *table, SectionMapList *sml, Section *section,
    u64 linked_sect_idx, u64 obj_idx, u64 sect_idx)
{
    LinkedSection *linked_section = &table->sections[linked_sect_idx];
    sml->section_map[obj_idx][sect_idx].linked_section = linked_sect_idx;
    sml->section_map[obj_idx][sect_idx].offset_adjust = linked_section->size;
    linked_section->size += section->size;
    linked_section->data = realloc(table->sections[linked_sect_idx].data, linked_section->size * sizeof(u8));
    memcpy(&linked_section->data[sml->section_map[obj_idx][sect_idx].offset_adjust], section->data, section->size * sizeof(u8));
}
