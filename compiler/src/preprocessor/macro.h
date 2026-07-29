#ifndef NA_16_MACRO_H
#define NA_16_MACRO_H

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

typedef struct MacroTable {
    Macro *entries;
    size_t count;
    size_t capacity;
} MacroTable;

Macro *macro_table_find(const MacroTable *table, const char *name);

bool macro_table_define(MacroTable *table, Macro macro);

void macro_table_undef(const MacroTable *table, const char *name);

#endif //NA_16_MACRO_H
