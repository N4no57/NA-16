.global _start

_start:
    push word bp ; save old bp
    mov bp, sp ; bp = sp

    call init_heap
    mov r1, 10
    call malloc

    mov r1, r0
    call free

    mov sp, bp ; sp = bp
    pop word bp ; restore old bp
    nop
    hlt

.include "lib.asm"

init_heap:
    ; u8 in_use
    ; u16 size
    ; heap_chunk *next
    mov r0, 0x4000
    mov byte [r0], 0
    add r0, r0, 1
    mov word [r0], 0x1000
    add r0, r0, 2
    mov word [r0], 0
    ret

malloc:
    ; r1 = size
    ; if (size == 0)
    test word r1, r1
    je malloc_ret_null
    ; struct heapchunk_t *chunk = heap.start;
    mov r0, 0x4000
malloc_while: ; while (chunk != NULL)
    test word r0, r0
    jz malloc_while_end
    ; if (!chunk->inuse && chunk->size >= size)
    mov byte r2, [r0]
    test r2, r2
    jne l0
    add r2, r0, 1
    mov word r3, [r2]
    cmp word r3, r1
    jb l0
    jmp malloc_while_end
l0:
    add r2, r0, 3 ; chunk = chunk->next
    mov word r0, [r2]
    jmp malloc_while
malloc_while_end:
    test word r0, r0 ; if (chunk == NULL)
    jz malloc_ret_null
    ; u16 leftover = chunk->size - size
    add r2, r0, 1
    mov word r2, [r2]
    sub r2, r2, r1
    ; if (leftover > MIN_CHUNK_SIZE)
    cmp r2, 9
    jbe l1
    ; struct heapchunk_t *new_chunk = (struct heapchunk_t *)((void *)chunk + sizeof(struct heapchunk_t) + size)
    mov r3, r0
    add r3, r3, 5
    add r3, r3, r1
    ; new_chunk->inuse = false
    mov byte [r3], 0
    ; new_chunk->size = leftover - sizeof(struct heapchunk_t)
    add r4, r3, 1
    sub r5, r2, 5
    mov word [r4], r5
    ; new_chunk->next = chunk->next
    add r2, r0, 3
    mov word r4, [r2]
    add r2, r3, 3
    mov word [r2], r4
    ; chunk->next = new_chunk
    add r2, r0, 3
    mov word [r2], r3
    ; chunk->size = size
    add r2, r0, 1
    mov word [r2], r1
l1:
    ; chunk->inuse = true;
    mov byte [r0], 1
    ; return (void *)chunk + sizeof(struct heapchunk_t)
    add r0, r0, 5
    ret
malloc_ret_null:
    ; return NULL;
    mov r0, 0
    ret

free:
    ; r1 = ptr
    test word r1, r1
    jz free_ret

    sub r1, r1, 5
    mov byte [r0], 0
free_ret:
    ret