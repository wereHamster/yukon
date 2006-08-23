
BITS 32

SECTION .text
global huffCompress
global huffDecompress

;; u32 *huffCompress(u32 *dst, u8 *src, u8 *end, u32 *tbl)
;;	[esp+ 4] u32 *dst,
;;	[esp+ 8] u8  *src,
;;	[esp+12] u8  *end,
;;	[esp+16] u32 *tbl);

huffCompress:
    push    edi
    push    esi
    push    ebp
    push    ebx
    push    edx

    mov     edi, [esp+20+ 4]
    mov     esi, [esp+20+ 8]
    mov     ebp, [esp+20+12]
    mov     ebx, [esp+20+16]

    mov     dl,-32             ; bits to go
    
.loop:
    xor     ecx,ecx
    mov     cl,byte [esi]      ; load byte
    mov     ecx, [ebx+4*ecx]   ; load huffman code/length
    add	    dl,cl              ; add code legth into 'dl'
    jl	    .nostore           ; enough space in the register?

    sub     cl,dl              ; no, put in what fits in and adjust 'cl'
    sub     dl,32
    shld    eax,ecx,cl
    add     cl,dl
    bswap   eax
    mov     [edi],eax
    add     edi,4
    
.nostore:                      ; put the code into 'eax'
    shld    eax,ecx,cl
    
    add     esi,1              ; advance source by one byte
    cmp     esi,ebp            ; loop...
    jnz     .loop

    cmp     dl,-32             ; extra bits to flush?
    jle    .noextra
    
    mov     cl,dl
    neg     cl
    shl     eax,cl
    bswap   eax
    mov     [edi],eax
    add     edi,4

.noextra:
    mov     eax,edi            ; how many dwords did we encode?
    
    pop     edx
    pop     ebx
    pop     ebp
    pop     esi
    pop     edi
	
    retn
