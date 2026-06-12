.section text
.global _start

table = 0x1002

_start:
    mov r0, handler
    mov r1, 0x100
    mov word r2, table
loop:
    mov word [r2], r0
    add r2, r2, 2
    sub r1, r1, 1
    test r1, r1
    jnz loop
    mov IVBR, table
    mov KSP, 0x1000
stop:
    int 0x8
    hlt
    hlt
    jmp stop

.dq 0, 0, 0, 0

handler:
    nop
    nop
    nop
    iret
