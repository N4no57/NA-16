.section text
.global _start

page_table_start = 0x1000

_start:
    mov r0, 0x0E
    xor r1, r1
    xor r2, r2, r2
    mov r3, page_table_start
    mov byte [r3+r2*4+1], r0
    add r0, r0, 0x20
    add r2, r2, 15
    mov byte [r3+r2*4+1], r0

    mov cr0, page_table_start

    mov r0, fr
    or r0, r0, 0x40 ; VME set
    mov fr, r0

    mov r0, 0xF000
    mov byte [r0], 10

    nop
    hlt
