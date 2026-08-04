#include "source.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char trigraph_replacement[] = {
    ['='] = '#',
    ['/'] = '\\',
    ['\''] = '^',
    ['('] = '[',
    [')'] = ']',
    ['!'] = '|',
    ['<'] = '{',
    ['>'] = '}',
    ['-'] = '~'
};

static void phase_1_normalise(SourceFile *source) {
    char *text = source->contents;
    size_t i = 0;

    while (i < source->length) {
        if (text[i] == '\r') {
            if (text[i+1] == '\n') {
                memmove(text+i, text+i+1, source->length-i);
                source->length--;
            } else {
                text[i] = '\n';
            }
        }

        if (text[i] == '?' && text[i+1] == '?') {
            const size_t start = i;
            i += 2;

            switch (text[i]) {
                case '=':
                case '/':
                case '\'':
                case '(':
                case ')':
                case '!':
                case '<':
                case '>':
                case '-':
                    text[start] = trigraph_replacement[(size_t)text[i]];
                    memmove(text+start+1, text+i+1, source->length-i);
                    source->length -= 2;
                    i = start;
                default:
                    break;
            }
        }

        i++;
    }
}

static void phase_2_splice(SourceFile *source) {
    char *text = source->contents;
    size_t i = 0;

    while (i < source->length) {
        if (text[i] == '\\' && text[i+1] == '\n') {
            memmove(text+i, text+i+2, source->length-i-1);
            source->length -= 2;
        }

        i++;
    }
}

Error source_file_load(SourceFile *source, const char *path) {
    FILE *f = fopen(path, "rb");

    if (!f) return ERROR_FILE_OPEN_FAILED;

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return ERROR_FILE_SEEK_FAILED;
    }

    source->length = (size_t)ftell(f);
    source->contents = malloc(source->length+1);

    if (source->contents == nullptr) {
        fclose(f);
        return ERROR_ALLOCATION_FAILED;
    }

    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return ERROR_FILE_SEEK_FAILED;
    }

    const size_t count = fread(source->contents, 1, source->length, f);

    if (count < source->length) {
        if (ferror(f)) {
            fclose(f);
            return ERROR_FILE_READ_FAILED;
        }

        if (feof(f)) {
            fclose(f);
            return ERROR_END_OF_FILE;
        }
    }

    fclose(f);

    source->contents[source->length] = '\0';
    source->path = strdup(path);

    phase_1_normalise(source);
    phase_2_splice(source);

    return ERROR_OK;
}

void source_file_destroy(SourceFile *source) {
    free(source->contents);
    free(source->path);
    source->contents = nullptr;
    source->path = nullptr;
    source->length = 0;
}

char *copy_string(const SourceSpan *span) {
    const size_t size = span->end.column - span->begin.column;
    char *str = malloc(size+1);

    if (!str) {
        return nullptr;
    }

    const char *start = &span->begin.file->contents[span->begin.offset];
    memcpy(str, start, size);
    str[size] = '\0';

    return str;
}