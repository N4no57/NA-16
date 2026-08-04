#ifndef NA_16_MACRO_H
#define NA_16_MACRO_H

#include <error.h>
#include <vector.h>

#include "../lexer/lexer.h"

typedef enum MacroKind {
    MACRO_OBJECT_LIKE,
    MACRO_FUNCTION_LIKE
} MacroKind;

typedef struct Macro {
    char *name;
    MacroKind kind;

    char **parameters;
    size_t parameter_count;
    bool variadic;

    PPToken *replacement;
    size_t replacement_count;

    SourceSpan definition_span;
} Macro;

Macro *macro_table_find(Vector *table, char *name);

Error macro_table_define(Vector *table, Macro macro);

void macro_table_undef(Vector *table, char *name);

#endif //NA_16_MACRO_H
