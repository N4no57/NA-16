#include "relocations.h"

#include <stdlib.h>
#include <string.h>

void init_RelocationTable(RelocationTable *list) {
    list->count = 0;
    list->size = 8;
    list->relocations = malloc(list->size * sizeof(Relocation));
}

void free_RelocationTable(RelocationTable *list) {
    free(list->relocations);
}

void relocation_push(RelocationTable *list, Relocation *relocation) {
    if (list->count >= list->size) {
        list->size *= 2;
        Relocation *tmp = realloc(list->relocations, list->size * sizeof(Relocation));
        if (tmp == NULL) {
            exit(1);
        }
        list->relocations = tmp;
    }

    list->relocations[list->count] = *relocation;
    list->count++;
}

Relocation *get_relocation(RelocationTable *list, char *name) {
    for (u64 i = 0; i < list->count; i++) {
        if (strcmp(list->relocations[i].name, name) == 0) {
            return &list->relocations[i];
        }
    }
    return nullptr;
}