#include "error.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

void print_snippet(Position pos) {
    if (!pos.source) return;

    fprintf(stderr, "      | ");

    u64 i = pos.idx - pos.column + 1;
    while (pos.source[i] != '\n' && pos.source[i] != '\0') {
        putc(pos.source[i], stderr);
        i++;
    }
    putc('\n', stderr);

    fprintf(stderr, "      | ");

    for (i = 1; i < pos.column; i++) {
        putc(' ', stderr);
    }

    fprintf(stderr, "^\n");
}

static void diag_vprint(Position pos, const char *kind, const char *fmt, va_list args) {
    fprintf(stderr, "NAAS: %s: %s:%llu:%llu: ",
            kind,
            pos.filename ? (char*)pos.filename : "<unknown>",
            pos.line,
            pos.column);

    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");

    print_snippet(pos);
}

void error(Position pos, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    diag_vprint(pos, "error", fmt, args);
    va_end(args);
    error_count++;
}

void fatal(Position pos, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    diag_vprint(pos, "fatal", fmt, args);
    va_end(args);

    exit(EXIT_FAILURE);
}
