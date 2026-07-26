.section text
.global _start

table     = 0x1002
kernel_pt = 0x2000
user_pt   = 0x2040

_start:
    mov sp, 0x1000
    mov r0, handler
    mov r1, 0x100
    mov word r2, table
loop:
    mov word [r2], r0
    add r2, r2, 2
    sub r1, r1, 1
    test r1, r1
    jnz loop

    mov word r2, kernel_pt     ; PTE destination
    mov word r3, 0x0E00        ; frame 0 + P/W/X
    mov word r4, 16

kernel_pt_loop:
    mov word [r2], r3       ; low 16 bits
    mov word [r2 + 2], 0    ; upper 16 bits

    add r2, r2, 4
    add r3, r3, 0x1000     ; next physical frame
    sub r4, r4, 1
    test r4, r4
    jnz kernel_pt_loop

    mov word r2, user_pt
    mov word r3, 0x0F00        ; frame 0 + P/W/X/U
    mov word r4, 16

user_pt_loop:
    mov word [r2], r3
    mov word [r2 + 2], 0

    add r2, r2, 4
    add r3, r3, 0x1000
    sub r4, r4, 1
    test r4, r4
    jnz user_pt_loop

    mov cr0, kernel_pt
    mov cr1, user_pt
    mov IVBR, table
    mov r0, FR
    or r0, r0, 0x40
    mov FR, r0
    mov KSP, 0x3000
    mov r0, FR
    or r0, r0, 0xD0
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
