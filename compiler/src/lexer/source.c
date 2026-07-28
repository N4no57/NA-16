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

bool source_file_load(SourceFile *source, const char *path) {
    FILE *f = fopen(path, "rb");

    if (!f) return false;

    fseek(f, 0, SEEK_END);
    source->length = (size_t)ftell(f);
    source->contents = malloc(source->length+1);
    if (source->contents == nullptr) {
        return false;
    }

    rewind(f);
    fread(source->contents, 1, source->length, f);
    fclose(f);

    source->contents[source->length] = '\0';
    source->path = strdup(path);

    phase_1_normalise(source);
    phase_2_splice(source);

    return true;
}

void source_file_destroy(SourceFile *source) {
    free(source->contents);
    free(source->path);
    source->contents = nullptr;
    source->path = nullptr;
    source->length = 0;
}
