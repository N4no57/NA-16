#ifndef NA_16_SOURCE_H
#define NA_16_SOURCE_H

#include <stdint.h>

typedef struct SourceFile {
    char *path;
    char *contents;
    size_t length;
} SourceFile;

typedef struct SourceLocation {
    const SourceFile *file;

    size_t offset;
    uint32_t line;
    uint32_t column;
} SourceLocation;

typedef struct SourceSpan {
    SourceLocation begin;
    SourceLocation end;
} SourceSpan;

bool source_file_load(SourceFile *source, const char *path);

void source_file_destroy(SourceFile *source);

#endif //NA_16_SOURCE_H
