#include "source.h"

#include <stdio.h>
#include <stdlib.h>

bool source_file_load(SourceFile *source, const char *path) {
    FILE *f = fopen(path, "r");

    if (!f) return false;

    fseek(f, 0, SEEK_END);
    source->length = (size_t)ftell(f);
    source->contents = malloc(source->length);
    fseek(f, 0, SEEK_SET);
    fread(source->contents, 1, source->length, f);
    fclose(f);

    return true;
}

void source_file_destroy(SourceFile *source) {
    free(source->contents);
    source->contents = nullptr;
    source->path = nullptr;
    source->length = 0;
}
