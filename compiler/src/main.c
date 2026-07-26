#include <stdio.h>
#include <stdlib.h>

#include "target.h"

static void print_integer_type(
    const char *name,
    const TargetIntegerLayout *type
)
{
    printf(
        "%-20s size=%u align=%u width=%u rank=%u signed=%s\n",
        name,
        (unsigned)type->object.size,
        (unsigned)type->object.alignment,
        (unsigned)type->width,
        (unsigned)type->rank,
        type->is_signed ? "yes" : "no"
    );
}

int main(void)
{
    const TargetInfo *target = target_na16();

    char error[256];

    if (!target_validate(target, error, sizeof(error))) {
        fprintf(stderr, "invalid target: %s\n", error);
        return EXIT_FAILURE;
    }

    printf("Target: %s\n", target->name);
    printf("CHAR_BIT: %u\n", (unsigned)target->char_bits);
    printf(
        "Byte order: %s\n",
        target->byte_order == TARGET_LITTLE_ENDIAN
            ? "little-endian"
            : "big-endian"
    );

    print_integer_type(
        "char",
        target_get_integer(target, TARGET_INT_CHAR)
    );

    print_integer_type(
        "short",
        target_get_integer(target, TARGET_INT_SHORT)
    );

    print_integer_type(
        "int",
        target_get_integer(target, TARGET_INT_INT)
    );

    print_integer_type(
        "long",
        target_get_integer(target, TARGET_INT_LONG)
    );

    print_integer_type(
        "long long",
        target_get_integer(target, TARGET_INT_LONG_LONG)
    );

    printf(
        "%-20s size=%u align=%u\n",
        "object pointer",
        (unsigned)target->object_pointer.size,
        (unsigned)target->object_pointer.alignment
    );

    return EXIT_SUCCESS;
}