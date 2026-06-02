#include "sections.h"

#include <stdlib.h>
#include <string.h>

Section defaultSections[] = {
    {"text", nullptr, 0, 0},
    {"data", nullptr, 0, 0},
    {"bss", nullptr, 0, 0},
    {"rodata", nullptr, 0, 0},
};

void init_SectionTable(SectionTable *list) {
    list->count = 4;
    list->size = 8;
    list->current = 0; // set to text section

    list->sections = malloc(list->size * sizeof(Section *));
    if (!list->sections) exit(1);

    memcpy(list->sections, defaultSections, sizeof(defaultSections));
}

void free_SectionTable(SectionTable *list) {
    free(list->sections);
}

void section_push(SectionTable *list, Section *section) {
    if (list->count >= list->size) {
        if (list->size == 0) {
            init_SectionTable(list);
        } else {
            list->size *= 2;
            Section *tmp = realloc(list->sections, list->size * sizeof(Section *));
            if (!tmp) {
                exit(1);
            }
            list->sections = tmp;
        }
    }

    memcpy(&list->sections[list->count], section, sizeof(Section));
    list->count++;
}

Section *get_section(SectionTable *list, char *name) {
    for (u64 i = 0; i < list->count; i++) {
        if (strcmp(list->sections[i].name, name) == 0) {
            return &list->sections[i];
        }
    }

    return nullptr;
}