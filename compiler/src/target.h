#ifndef NACC_TARGET_H
#define NACC_TARGET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum TargetByteOrder {
    TARGET_LITTLE_ENDIAN,
    TARGET_BIG_ENDIAN
} TargetByteOrder;

typedef enum TargetSignedRepresentation {
    TARGET_SIGN_MAGNITUDE,
    TARGET_ONES_COMPLEMENT,
    TARGET_TWOS_COMPLEMENT
} TargetSignedRepresentation;

typedef enum TargetIntegerKind {
    TARGET_INT_BOOL,

    TARGET_INT_CHAR,
    TARGET_INT_SIGNED_CHAR,
    TARGET_INT_UNSIGNED_CHAR,

    TARGET_INT_SHORT,
    TARGET_INT_UNSIGNED_SHORT,

    TARGET_INT_INT,
    TARGET_INT_UNSIGNED_INT,

    TARGET_INT_LONG,
    TARGET_INT_UNSIGNED_LONG,

    TARGET_INT_LONG_LONG,
    TARGET_INT_UNSIGNED_LONG_LONG,

    TARGET_INTEGER_KIND_COUNT
} TargetIntegerKind;

typedef enum TargetFloatingKind {
    TARGET_FLOAT_FLOAT,
    TARGET_FLOAT_DOUBLE,
    TARGET_FLOAT_LONG_DOUBLE,

    TARGET_FLOATING_KIND_COUNT
} TargetFloatingKind;

typedef struct TargetObjectLayout {
    uint16_t size;
    uint16_t alignment;
} TargetObjectLayout;

typedef struct TargetIntegerLayout {
    TargetObjectLayout object;

    /*
     * Number of value and sign bits in the representation.
     * For NA-16 this is currently the same as size * CHAR_BIT.
     */
    uint16_t width;

    /*
     * C integer-conversion rank.
     *
     * Only relative ordering matters:
     *
     * _Bool < char < short < int < long < long long
     */
    uint8_t rank;

    bool is_signed;
} TargetIntegerLayout;

typedef struct TargetFloatingLayout {
    TargetObjectLayout object;

    /*
     * These describe the intended floating-point format.
     * They include the implicit leading significand bit.
     */
    uint16_t significand_bits;
    uint16_t exponent_bits;
} TargetFloatingLayout;

typedef struct TargetInfo {
    const char *name;

    uint16_t char_bits;

    TargetByteOrder byte_order;
    TargetSignedRepresentation signed_representation;

    bool plain_char_is_signed;
    bool signed_right_shift_is_arithmetic;

    TargetIntegerLayout integers[TARGET_INTEGER_KIND_COUNT];
    TargetFloatingLayout floating[TARGET_FLOATING_KIND_COUNT];

    TargetObjectLayout object_pointer;
    TargetObjectLayout function_pointer;

    uint64_t null_object_pointer;
    uint64_t null_function_pointer;

    uint16_t maximum_object_alignment;
    uint16_t stack_alignment;
} TargetInfo;

/*
 * Returns the built-in NA-16 target description.
 *
 * The returned object has static storage duration and must not be modified.
 */
const TargetInfo *target_na16(void);

const TargetIntegerLayout *target_get_integer(
    const TargetInfo *target,
    TargetIntegerKind kind
);

const TargetFloatingLayout *target_get_floating(
    const TargetInfo *target,
    TargetFloatingKind kind
);

/*
 * Checks that the target description is internally consistent.
 *
 * On failure, an explanation is placed in error_buffer when one is supplied.
 */
bool target_validate(
    const TargetInfo *target,
    char *error_buffer,
    size_t error_buffer_size
);

#endif