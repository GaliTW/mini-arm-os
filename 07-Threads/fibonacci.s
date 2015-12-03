    .syntax unified
    .arch armv7-m
    .text
    .align 2
    .thumb
    .thumb_func

    .global _fibonacci
    .type _fibonacci, function

_fibonacci:
                          @ r0(rst), r4(pre), r5(i), r6(sum)
    push {r4, r5, r6, lr} @ PROLOG

    mov r5, r0            @ r5 = x 
    mov r4, #1            @ pre = 1
    mov r0, #0            @ rst = 0

.loop:
    subs r5, r5, #1
    blt .end              @ if ( i < 0 ) goto .end, return rst

    add r6, r4, r0        @ sum = rst + pre
    mov r4, r0            @ pre = rst
    mov r0, r6            @ rst = sum

    b .loop

.end:
    pop {r4, r5, r6, pc}  @ EPILOG

    .size _fibonacci, .-_fibonacci
    .end
