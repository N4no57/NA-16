#include "../lexer/target.h"

#include <stdio.h>

enum {
    RANK_BOOL = 0,
    RANK_CHAR = 1,
    RANK_SHORT = 2,
    RANK_INT = 3,
    RANK_LONG = 4,
    RANK_LONG_LONG = 5
};

#define INTEGER_LAYOUT(bytes, alignment_, width_, rank_, signed_) \
    {                                                            \
        .object = {                                               \
            .size = (bytes),                                      \
            .alignment = (alignment_)                             \
        },                                                        \
        .width = (width_),                                        \
        .rank = (rank_),                                          \
        .is_signed = (signed_)                                    \
    }

static const TargetInfo na16_target = {
    .name = "na16",

    .char_bits = 8,

    .byte_order = TARGET_LITTLE_ENDIAN,
    .signed_representation = TARGET_TWOS_COMPLEMENT,

    .plain_char_is_signed = true,
    .signed_right_shift_is_arithmetic = true,

    .integers = {
        [TARGET_INT_BOOL] = INTEGER_LAYOUT(
            1, 1, 8, RANK_BOOL, false
        ),

        [TARGET_INT_CHAR] = INTEGER_LAYOUT(
            1, 1, 8, RANK_CHAR, true
        ),

        [TARGET_INT_SIGNED_CHAR] = INTEGER_LAYOUT(
            1, 1, 8, RANK_CHAR, true
        ),

        [TARGET_INT_UNSIGNED_CHAR] = INTEGER_LAYOUT(
            1, 1, 8, RANK_CHAR, false
        ),

        [TARGET_INT_SHORT] = INTEGER_LAYOUT(
            2, 2, 16, RANK_SHORT, true
        ),

        [TARGET_INT_UNSIGNED_SHORT] = INTEGER_LAYOUT(
            2, 2, 16, RANK_SHORT, false
        ),

        [TARGET_INT_INT] = INTEGER_LAYOUT(
            2, 2, 16, RANK_INT, true
        ),

        [TARGET_INT_UNSIGNED_INT] = INTEGER_LAYOUT(
            2, 2, 16, RANK_INT, false
        ),

        [TARGET_INT_LONG] = INTEGER_LAYOUT(
            4, 2, 32, RANK_LONG, true
        ),

        [TARGET_INT_UNSIGNED_LONG] = INTEGER_LAYOUT(
            4, 2, 32, RANK_LONG, false
        ),

        [TARGET_INT_LONG_LONG] = INTEGER_LAYOUT(
            8, 2, 64, RANK_LONG_LONG, true
        ),

        [TARGET_INT_UNSIGNED_LONG_LONG] = INTEGER_LAYOUT(
            8, 2, 64, RANK_LONG_LONG, false
        )
    },

    .floating = {
        [TARGET_FLOAT_FLOAT] = {
            .object = {
                .size = 4,
                .alignment = 2
            },
            .significand_bits = 24,
            .exponent_bits = 8
        },

        [TARGET_FLOAT_DOUBLE] = {
            .object = {
                .size = 8,
                .alignment = 2
            },
            .significand_bits = 53,
            .exponent_bits = 11
        },

        [TARGET_FLOAT_LONG_DOUBLE] = {
            .object = {
                .size = 8,
                .alignment = 2
            },
            .significand_bits = 53,
            .exponent_bits = 11
        }
    },

    .object_pointer = {
        .size = 2,
        .alignment = 2
    },

    .function_pointer = {
        .size = 2,
        .alignment = 2
    },

    .null_object_pointer = 0,
    .null_function_pointer = 0,

    .maximum_object_alignment = 2,
    .stack_alignment = 2
};

#undef INTEGER_LAYOUT

const TargetInfo *target_na16(void)
{
    return &na16_target;
}

const TargetIntegerLayout *target_get_integer(
    const TargetInfo *target,
    TargetIntegerKind kind
)
{
    if (target == nullptr) {
        return nullptr;
    }

    if (kind >= TARGET_INTEGER_KIND_COUNT) {
        return nullptr;
    }

    return &target->integers[kind];
}

const TargetFloatingLayout *target_get_floating(
    const TargetInfo *target,
    TargetFloatingKind kind
)
{
    if (target == nullptr) {
        return nullptr;
    }

    if (kind >= TARGET_FLOATING_KIND_COUNT) {
        return nullptr;
    }

    return &target->floating[kind];
}

static bool report_validation_error(
    char *buffer,
    size_t buffer_size,
    const char *message
)
{
    if (buffer != nullptr && buffer_size > 0) {
        snprintf(buffer, buffer_size, "%s", message);
    }

    return false;
}

bool target_validate(
    const TargetInfo *target,
    char *error_buffer,
    size_t error_buffer_size
)
{
    if (target == nullptr) {
        return report_validation_error(
            error_buffer,
            error_buffer_size,
            "target description is null"
        );
    }

    if (target->name == nullptr || target->name[0] == '\0') {
        return report_validation_error(
            error_buffer,
            error_buffer_size,
            "target has no name"
        );
    }

    if (target->char_bits < 8) {
        return report_validation_error(
            error_buffer,
            error_buffer_size,
            "CHAR_BIT must be at least 8"
        );
    }

    if (target->object_pointer.size == 0) {
        return report_validation_error(
            error_buffer,
            error_buffer_size,
            "object pointer size cannot be zero"
        );
    }

    if (target->object_pointer.alignment == 0) {
        return report_validation_error(
            error_buffer,
            error_buffer_size,
            "object pointer alignment cannot be zero"
        );
    }

    if (target->stack_alignment == 0) {
        return report_validation_error(
            error_buffer,
            error_buffer_size,
            "stack alignment cannot be zero"
        );
    }

    for (size_t i = 0; i < TARGET_INTEGER_KIND_COUNT; ++i) {
        const TargetIntegerLayout *integer = &target->integers[i];

        if (integer->object.size == 0) {
            return report_validation_error(
                error_buffer,
                error_buffer_size,
                "integer type has zero size"
            );
        }

        if (integer->object.alignment == 0) {
            return report_validation_error(
                error_buffer,
                error_buffer_size,
                "integer type has zero alignment"
            );
        }

        if (integer->width == 0) {
            return report_validation_error(
                error_buffer,
                error_buffer_size,
                "integer type has zero width"
            );
        }
    }

    /*
     * C requires sizeof(char), sizeof(signed char), and
     * sizeof(unsigned char) to all equal one.
     */
    if (
        target->integers[TARGET_INT_CHAR].object.size != 1 ||
        target->integers[TARGET_INT_SIGNED_CHAR].object.size != 1 ||
        target->integers[TARGET_INT_UNSIGNED_CHAR].object.size != 1
    ) {
        return report_validation_error(
            error_buffer,
            error_buffer_size,
            "all character types must have size one"
        );
    }

    if (
        target->integers[TARGET_INT_SHORT].rank >=
        target->integers[TARGET_INT_INT].rank
    ) {
        return report_validation_error(
            error_buffer,
            error_buffer_size,
            "short must have lower conversion rank than int"
        );
    }

    if (
        target->integers[TARGET_INT_INT].rank >=
        target->integers[TARGET_INT_LONG].rank
    ) {
        return report_validation_error(
            error_buffer,
            error_buffer_size,
            "int must have lower conversion rank than long"
        );
    }

    if (
        target->integers[TARGET_INT_LONG].rank >=
        target->integers[TARGET_INT_LONG_LONG].rank
    ) {
        return report_validation_error(
            error_buffer,
            error_buffer_size,
            "long must have lower conversion rank than long long"
        );
    }

    if (error_buffer != nullptr && error_buffer_size > 0) {
        error_buffer[0] = '\0';
    }

    return true;
}