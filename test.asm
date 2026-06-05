.section text
.global _start
.extern memcpy

_start:
    push word bp ; save old bp
    mov bp, sp ; bp = sp

    mov r1, 0x2000
    xor r2, r2, r2
    mov r3, 80
    call memcpy

    call init_heap
    mov r1, 10
    call malloc

    mov r1, r0
    call free

    mov sp, bp ; sp = bp
    pop word bp ; restore old bp
    nop
    hlt

.section heap

init_heap:
    ; u8 in_use
    ; u16 size
    ; heap_chunk *next
    mov r0, 0x4000
    mov byte [r0], 0
    mov word [r0+1], 0x1000
    mov word [r0+3], 0
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
    jne malloc_next
    mov word r3, [r0+1]
    cmp word r3, r1
    jb malloc_next
    jmp malloc_while_end
malloc_next:
    mov word r0, [r0+3]
    jmp malloc_while
malloc_while_end:
    test word r0, r0 ; if (chunk == NULL)
    jz malloc_ret_null
    ; u16 leftover = chunk->size - size
    mov word r2, [r0+1]
    sub r2, r2, r1
    ; if (leftover > MIN_CHUNK_SIZE)
    cmp r2, 9
    jb malloc_no_split
    ; struct heapchunk_t *new_chunk = (struct heapchunk_t *)((void *)chunk + sizeof(struct heapchunk_t) + size)
    lea r3, [r0+r1+5]
    ; new_chunk->inuse = false
    mov byte [r3], 0
    ; new_chunk->size = leftover - sizeof(struct heapchunk_t)
    sub r5, r2, 5
    mov word [r3+1], r5
    ; new_chunk->next = chunk->next
    mov word r4, [r0+3]
    mov word [r3+3], r4
    ; chunk->next = new_chunk
    mov word [r0+3], r3
    ; chunk->size = size
    mov word [r0+1], r1
malloc_no_split:
    ; chunk->inuse = true;
    mov byte [r0], 1
    ; return (void *)chunk + sizeof(struct heapchunk_t)
    lea r0, [r0+5]
    ret
malloc_ret_null:
    ; return NULL;
    xor r0, r0, r0
    ret

free:
    ; r1 = ptr
    test word r1, r1
    jz free_ret

    mov byte [r1-5], 0

    ; if (chunk->next && !chunk->next->in_use)
    mov byte r2, [r1-2]
    test word r2, r2
    jz free_ret
    mov byte r3, [r2]
    test r3, r3
    jnz free_ret
    mov r3, 5 ; chunk->size += sizeof(header) + chunk->next->size;
    add word r3, r3, [r2+1]
    add word r3, r3, [r1-4]
    mov word [r1-4], r3
    mov word r3, [r2+3] ; chunk->next = chunk->next->next;
    mov word [r1-2], r3

free_ret:
    ret