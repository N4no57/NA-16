.section lib
.global memcpy

memcpy:
    ; r1 = dest
    ; r2 = src
    ; r3 = count
    mov r0, r1
    mov r4, 0
memcpy_loop:
    cmp word r3, r4
    je memcpy_ret
    mov byte r6, [r2]
    mov byte [r1], r6
    add r2, 1, r2
    add r1, 1, r1
    add r4, 1, r4
    jmp memcpy_loop
memcpy_ret:
    ret