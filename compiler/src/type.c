#include "type.h"

#include <stdlib.h>

static const CType builtin_types[] = {
    [CTYPE_VOID] = {
        .kind = CTYPE_VOID,
    },
    [CTYPE_BOOL] = {
        .kind = CTYPE_BOOL,
    },
    [CTYPE_CHAR] = {
        .kind = CTYPE_CHAR,
    },
    [CTYPE_SIGNED_CHAR] = {
        .kind = CTYPE_SIGNED_CHAR,
    },
    [CTYPE_UNSIGNED_CHAR] = {
        .kind = CTYPE_UNSIGNED_CHAR,
    },
    [CTYPE_SHORT] = {
        .kind = CTYPE_SHORT,
    },
    [CTYPE_UNSIGNED_SHORT] = {
        .kind = CTYPE_UNSIGNED_SHORT,
    },
    [CTYPE_INT] = {
        .kind = CTYPE_INT,
    },
    [CTYPE_UNSIGNED_INT] = {
        .kind = CTYPE_UNSIGNED_INT,
    },
    [CTYPE_LONG] = {
        .kind = CTYPE_LONG,
    },
    [CTYPE_UNSIGNED_LONG] = {
        .kind = CTYPE_UNSIGNED_LONG,
    },
    [CTYPE_LONG_LONG] = {
        .kind = CTYPE_LONG_LONG,
    },
    [CTYPE_UNSIGNED_LONG_LONG] = {
        .kind = CTYPE_UNSIGNED_LONG_LONG,
    },
    [CTYPE_FLOAT] = {
        .kind = CTYPE_FLOAT,
    },
    [CTYPE_DOUBLE] = {
        .kind = CTYPE_DOUBLE,
    },
    [CTYPE_LONG_DOUBLE] = {
        .kind = CTYPE_LONG_DOUBLE,
    }
};

const CType *ctype_builtin(CTypeKind kind) {
    if (kind > CTYPE_LONG_DOUBLE) {
        return nullptr;
    }

    return &builtin_types[kind];
}

const CType *ctype_pointer_to(const CType *base) {
    if (base == nullptr) {
        return nullptr;
    }

    CType *type = malloc(sizeof(CType));

    if (type == nullptr) {
        return nullptr;
    }

    type->kind = CTYPE_POINTER;
    type->data.pointer.reference = base;

    return type;
}

uint16_t ctype_size(const CType *type, const TargetInfo *target) {
    if (type == NULL || target == NULL) {
        return 0;
    }

    switch (type->kind) {
        case CTYPE_BOOL:
            return target->integers[
                TARGET_INT_BOOL
            ].object.size;

        case CTYPE_CHAR:
            return target->integers[
                TARGET_INT_CHAR
            ].object.size;

        case CTYPE_SIGNED_CHAR:
            return target->integers[
                TARGET_INT_SIGNED_CHAR
            ].object.size;

        case CTYPE_UNSIGNED_CHAR:
            return target->integers[
                TARGET_INT_UNSIGNED_CHAR
            ].object.size;

        case CTYPE_SHORT:
            return target->integers[
                TARGET_INT_SHORT
            ].object.size;

        case CTYPE_UNSIGNED_SHORT:
            return target->integers[
                TARGET_INT_UNSIGNED_SHORT
            ].object.size;

        case CTYPE_INT:
            return target->integers[
                TARGET_INT_INT
            ].object.size;

        case CTYPE_UNSIGNED_INT:
            return target->integers[
                TARGET_INT_UNSIGNED_INT
            ].object.size;

        case CTYPE_LONG:
            return target->integers[
                TARGET_INT_LONG
            ].object.size;

        case CTYPE_UNSIGNED_LONG:
            return target->integers[
                TARGET_INT_UNSIGNED_LONG
            ].object.size;

        case CTYPE_LONG_LONG:
            return target->integers[
                TARGET_INT_LONG_LONG
            ].object.size;

        case CTYPE_UNSIGNED_LONG_LONG:
            return target->integers[
                TARGET_INT_UNSIGNED_LONG_LONG
            ].object.size;

        case CTYPE_FLOAT:
            return target->floating[
                TARGET_FLOAT_FLOAT
            ].object.size;

        case CTYPE_DOUBLE:
            return target->floating[
                TARGET_FLOAT_DOUBLE
            ].object.size;

        case CTYPE_LONG_DOUBLE:
            return target->floating[
                TARGET_FLOAT_LONG_DOUBLE
            ].object.size;

        case CTYPE_POINTER:
            return target->object_pointer.size;

        case CTYPE_VOID:
            return 0;
    }

    return 0;
}

uint16_t ctype_alignment(const CType *type, const TargetInfo *target) {
    if (type == NULL || target == NULL) {
        return 0;
    }

    switch (type->kind) {
        case CTYPE_BOOL:
            return target->integers[
                TARGET_INT_BOOL
            ].object.alignment;

        case CTYPE_CHAR:
            return target->integers[
                TARGET_INT_CHAR
            ].object.alignment;

        case CTYPE_SIGNED_CHAR:
            return target->integers[
                TARGET_INT_SIGNED_CHAR
            ].object.alignment;

        case CTYPE_UNSIGNED_CHAR:
            return target->integers[
                TARGET_INT_UNSIGNED_CHAR
            ].object.alignment;

        case CTYPE_SHORT:
            return target->integers[
                TARGET_INT_SHORT
            ].object.alignment;

        case CTYPE_UNSIGNED_SHORT:
            return target->integers[
                TARGET_INT_UNSIGNED_SHORT
            ].object.alignment;

        case CTYPE_INT:
            return target->integers[
                TARGET_INT_INT
            ].object.alignment;

        case CTYPE_UNSIGNED_INT:
            return target->integers[
                TARGET_INT_UNSIGNED_INT
            ].object.alignment;

        case CTYPE_LONG:
            return target->integers[
                TARGET_INT_LONG
            ].object.alignment;

        case CTYPE_UNSIGNED_LONG:
            return target->integers[
                TARGET_INT_UNSIGNED_LONG
            ].object.alignment;

        case CTYPE_LONG_LONG:
            return target->integers[
                TARGET_INT_LONG_LONG
            ].object.alignment;

        case CTYPE_UNSIGNED_LONG_LONG:
            return target->integers[
                TARGET_INT_UNSIGNED_LONG_LONG
            ].object.alignment;

        case CTYPE_FLOAT:
            return target->floating[
                TARGET_FLOAT_FLOAT
            ].object.alignment;

        case CTYPE_DOUBLE:
            return target->floating[
                TARGET_FLOAT_DOUBLE
            ].object.alignment;

        case CTYPE_LONG_DOUBLE:
            return target->floating[
                TARGET_FLOAT_LONG_DOUBLE
            ].object.alignment;

        case CTYPE_POINTER:
            return target->object_pointer.size;

        case CTYPE_VOID:
            return 0;
    }

    return 0;
}

bool ctype_is_integer(const CType *type) {
    if (type == nullptr) {
        return false;
    }

    switch (type->kind) {
        case CTYPE_BOOL:
        case CTYPE_CHAR:
        case CTYPE_SIGNED_CHAR:
        case CTYPE_UNSIGNED_CHAR:
        case CTYPE_SHORT:
        case CTYPE_UNSIGNED_SHORT:
        case CTYPE_INT:
        case CTYPE_UNSIGNED_INT:
        case CTYPE_LONG:
        case CTYPE_UNSIGNED_LONG:
        case CTYPE_LONG_LONG:
        case CTYPE_UNSIGNED_LONG_LONG:
            return true;

        default:
            return false;
    }
}

bool ctype_is_floating(const CType *type) {
    if (type == NULL) {
        return false;
    }

    switch (type->kind) {
        case CTYPE_FLOAT:
        case CTYPE_DOUBLE:
        case CTYPE_LONG_DOUBLE:
            return true;

        default:
            return false;
    }
}

bool ctype_is_arithmetic(const CType *type) {
    if (type == nullptr) {
        return false;
    }

    return ctype_is_integer(type) ||
           ctype_is_floating(type);
}

bool ctype_is_scalar(const CType *type) {
    if (type == nullptr) {
        return false;
    }

    return ctype_is_arithmetic(type) || type->kind == CTYPE_POINTER;
}

bool ctype_is_complete(const CType *type) {
    if (type == nullptr) {
        return false;
    }

    return type->kind != CTYPE_VOID;
}
