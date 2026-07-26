#ifndef NA_16_TYPE_H
#define NA_16_TYPE_H

#include "target/target.h"

typedef enum CTypeKind {
    CTYPE_VOID,
    CTYPE_BOOL,

    CTYPE_CHAR,
    CTYPE_SIGNED_CHAR,
    CTYPE_UNSIGNED_CHAR,

    CTYPE_SHORT,
    CTYPE_UNSIGNED_SHORT,

    CTYPE_INT,
    CTYPE_UNSIGNED_INT,

    CTYPE_LONG,
    CTYPE_UNSIGNED_LONG,

    CTYPE_LONG_LONG,
    CTYPE_UNSIGNED_LONG_LONG,

    CTYPE_FLOAT,
    CTYPE_DOUBLE,
    CTYPE_LONG_DOUBLE,

    CTYPE_POINTER
} CTypeKind;

typedef enum CTypeQualifier {
    CTYPE_QUALIFIER_NONE     = 0,
    CTYPE_QUALIFIER_CONST    = 1 << 0,
    CTYPE_QUALIFIER_VOLATILE = 1 << 1,
    CTYPE_QUALIFIER_RESTRICT = 1 << 2
} CTypeQualifier;

typedef struct CType CType;

struct CType {
    CTypeKind kind;
    unsigned qualifiers;

    union {
        struct {
            const CType *reference;
        } pointer;
    } data;
};

const CType *ctype_builtin(CTypeKind kind);
CType *ctype_create_pointer_to(const CType *base);

uint16_t ctype_size(const CType *type, const TargetInfo *target);
uint16_t ctype_alignment(const CType *type, const TargetInfo *target);

bool ctype_is_integer(const CType *type);
bool ctype_is_floating(const CType *type);
bool ctype_is_arithmetic(const CType *type);
bool ctype_is_scalar(const CType *type);
bool ctype_is_complete(const CType *type);

#endif //NA_16_TYPE_H
