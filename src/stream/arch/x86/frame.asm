
SECTION .text

; [esp+ 4] : buf
; [esp+ 8] : width
; [esp+12] : height
global streamFrameResample: function
streamFrameResample:
    push    edi
    push    esi
    push    edx
    push    ecx

    mov     edi,[esp+20]
    mov     esi,[esp+24]
    mov     edx,[esp+28]

    imul    edx,esi
    lea     ecx,[edi+edx*4]
    mov     eax,edi

    pxor    mm0,mm0

.L1:
    lea     edx,[eax+esi*4]

.L2:
    movd      mm1,[eax]
    punpcklbw mm1,mm0
    movd      mm2,[eax+4]
    punpcklbw mm2,mm0
    movd      mm3,[eax+esi*4]
    punpcklbw mm3,mm0
    movd      mm4,[eax+esi*4+4]
    punpcklbw mm4,mm0

    paddusw   mm1,mm2
    paddusw   mm1,mm3
    paddusw   mm1,mm4
    psrlw     mm1,2
    packuswb  mm1,mm0
    movd      [edi],mm1
    
    add     eax,8
    add     edi,4

    cmp     eax,edx
    jne    .L2

    lea     eax,[eax+esi*4]
    cmp     eax,ecx
    jne    .L1

    pop     ecx
    pop     edx
    pop     esi
    pop     edi

    emms
    ret


; [esp+ 4] : out[3]
; [esp+ 8] : in
; [esp+12] : width
; [esp+16] : height
global streamFrameConvert: function
streamFrameConvert:
    push    edi
    push    esi
    push    edx
    push    ecx
    push    ebp
    push    ebx

    mov     esi,[esp+32]
    mov     edx,[esp+36]
    mov     ecx,[esp+40]

    imul    ecx,edx
    lea     ebx,[esi+ecx*4]

    sub     esp,12
    mov     edi,[esp+40]

    mov     eax,[edi+0]
    mov     [esp+0],eax
    mov     eax,[edi+4]
    mov     [esp+4],eax
    mov     eax,[edi+8]
    mov     [esp+8],eax

    call   .L1
    dw    16,      157,        47,      0
    dw   112,      -87,       -26,      0
    dw   -10,     -102,       112,      0
.L1:
    pop     eax
    sub     esp,24
    
    movq    mm7,[eax]
    movq    [esp],mm7
    movq    mm7,[eax+8]
    movq    [esp+8],mm7
    movq    mm7,[eax+16]
    movq    [esp+16],mm7

    pxor    mm7,mm7

.L2:
    lea     ecx,[esi+edx*4]

.L3:
    movd      mm1,[esi]
    punpcklbw mm1,mm7
    movd      mm2,[esi+4]
    punpcklbw mm2,mm7
    movd      mm3,[esi+edx*4]
    punpcklbw mm3,mm7
    movd      mm4,[esi+edx*4+4]
    punpcklbw mm4,mm7
    movq      mm5,mm1
    paddw     mm5,mm2
    paddw     mm5,mm3
    paddw     mm5,mm4

    movq    mm6,[esp]

    pmaddwd   mm1,mm6
    movq      mm0,mm1
    psrlq     mm0,32
    paddd     mm1,mm0
    movd      eax,mm1
    shr       eax,8
    add       eax,16

    mov     edi,[esp+24]
    mov     [edi],al
    
    pmaddwd   mm2,mm6
    movq      mm0,mm2
    psrlq     mm0,32
    paddd     mm2,mm0
    movd      eax,mm2
    shr       eax,8
    add       eax,16
    
    mov     [edi+1],al
    add     edi,2
    mov     [esp+24],edi
    lea     edi,[edi+edx-2]

    pmaddwd   mm3,mm6
    movq      mm0,mm3
    psrlq     mm0,32
    paddd     mm3,mm0
    movd      eax,mm3
    shr       eax,8
    add       eax,16

    mov     [edi],al
    
    pmaddwd   mm4,mm6
    movq      mm0,mm4
    psrlq     mm0,32
    paddd     mm4,mm0
    movd      eax,mm4
    shr       eax,8
    add       eax,16

    mov     [edi+1],al
    mov     edi,[esp+28]
    movq    mm6,[esp+8]

    pmaddwd   mm6,mm5
    movq      mm0,mm6
    psrlq     mm0,32
    paddd     mm6,mm0
    movd      eax,mm6
    shr       eax,10
    add       eax,128

    mov     [edi],al
    add     edi,1
    mov     [esp+28],edi
    mov     edi,[esp+32]
    movq    mm6,[esp+16]

    pmaddwd   mm6,mm5
    movq      mm0,mm6
    psrlq     mm0,32
    paddd     mm6,mm0
    movd      eax,mm6
    shr       eax,10
    add       eax,128

    mov     [edi],al
    add     edi,1
    mov     [esp+32],edi
    
    add     esi,8
    
    cmp     esi,ecx
    jne    .L3

    lea     esi,[esi+edx*4]
    cmp     esi,ebx
    je     .L4

    mov     edi,[esp+24]
    add     edi,edx
    mov     [esp+24],edi
    jmp    .L2

.L4:
    add     esp,36

    pop     ebx
    pop     ebp
    pop     ecx
    pop     edx
    pop     esi
    pop     edi

    emms
    ret

SECTION ".note.GNU-stack" noalloc noexec nowrite progbits
