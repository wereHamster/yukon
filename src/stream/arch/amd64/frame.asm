
SECTION .rodata
ALIGN 16
yMul: dw    16,      157,        47,      0
uMul: dw   112,      -87,       -26,      0
vMul: dw   -10,     -102,       112,      0

SECTION .text

; rdi : buf
; rsi : width
; rdx : height
global streamFrameResample: function
streamFrameResample:
    imul    rdx,rsi
    lea     rcx,[rdi+rdx*4]
    mov     rax,rdi

    pxor    mm0,mm0

.L1:
    lea     rdx,[rax+rsi*4]

.L2:
    movd      mm1,[rax]
    punpcklbw mm1,mm0
    movd      mm2,[rax+4]
    punpcklbw mm2,mm0
    movd      mm3,[rax+rsi*4]
    punpcklbw mm3,mm0
    movd      mm4,[rax+rsi*4+4]
    punpcklbw mm4,mm0

    paddusw   mm1,mm2
    paddusw   mm1,mm3
    paddusw   mm1,mm4
    psrlw     mm1,2
    packuswb  mm1,mm0
    movd      [rdi],mm1

    add     rax,8
    add     rdi,4

    cmp     rax,rdx
    jne    .L2

    lea     rax,[rax+rsi*4]
    cmp     rax,rcx
    jne    .L1

    emms
    ret


; rdi : out[3]
; rsi : in
; rdx : width
; rcx : height
global streamFrameConvert: function
streamFrameConvert:
    push    r12
    push    r13
    push    r14
    push    r15

    imul    rcx,rdx
    lea     r15,[rsi+rcx*4]

    mov     r12,[rdi]
    mov     r13,[rdi+8]
    mov     r14,[rdi+16]

    pxor    mm7,mm7

.L1:
    lea     rcx,[rsi+rdx*4]

.L2:
    movd      mm1,[rsi]
    punpcklbw mm1,mm7
    movd      mm2,[rsi+4]
    punpcklbw mm2,mm7
    movd      mm3,[rsi+rdx*4]
    punpcklbw mm3,mm7
    movd      mm4,[rsi+rdx*4+4]
    punpcklbw mm4,mm7
    movq      mm5,mm1
    paddw     mm5,mm2
    paddw     mm5,mm3
    paddw     mm5,mm4

    movq      mm6,[yMul wrt rip]
    pmaddwd   mm1,mm6
    movq      mm0,mm1
    psrlq     mm0,32
    paddd     mm1,mm0
    movd      eax,mm1
    shr       eax,8
    add       eax,16
    mov       [r12],al

    pmaddwd   mm2,mm6
    movq      mm0,mm2
    psrlq     mm0,32
    paddd     mm2,mm0
    movd      eax,mm2
    shr       eax,8
    add       eax,16
    mov       [r12+1],al

    pmaddwd   mm3,mm6
    movq      mm0,mm3
    psrlq     mm0,32
    paddd     mm3,mm0
    movd      eax,mm3
    shr       eax,8
    add       eax,16
    mov       [r12+rdx],al

    pmaddwd   mm4,mm6
    movq      mm0,mm4
    psrlq     mm0,32
    paddd     mm4,mm0
    movd      eax,mm4
    shr       eax,8
    add       eax,16
    mov       [r12+rdx+1],al

    movq      mm6,[uMul wrt rip]
    pmaddwd   mm6,mm5
    movq      mm0,mm6
    psrlq     mm0,32
    paddd     mm6,mm0
    movd      eax,mm6
    shr       eax,10
    add       eax,128
    mov       [r13],al

    movq      mm6,[vMul wrt rip]
    pmaddwd   mm6,mm5
    movq      mm0,mm6
    psrlq     mm0,32
    paddd     mm6,mm0
    movd      eax,mm6
    shr       eax,10
    add       eax,128
    mov       [r14],al

    add     rsi,8

    add     r12,2
    add     r13,1
    add     r14,1

    cmp     rsi,rcx
    jne    .L2

    lea     rsi,[rsi+rdx*4]
    cmp     r15,rsi
    je     .L3

    lea     r12,[r12+rdx]
    jmp    .L1

.L3:
    pop     r15
    pop     r14
    pop     r13
    pop     r12

    emms
    ret

SECTION ".note.GNU-stack" noalloc noexec nowrite progbits
