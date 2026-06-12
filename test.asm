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
    mov r0, FR
    or r0, r0, 0x90
    push word r0
    push word user_prog
    iret
stop:
    hlt
    hlt
    hlt
    jmp stop

.dq 0, 0, 0, 0

handler:
    nop
    nop
    nop
    iret

user_prog:
    mov r0, 0x10
    mov r1, 0x20
    int 0x80
    hlt
    hlt
    hlt
