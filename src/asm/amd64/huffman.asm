
BITS 64

SECTION .text
global huffCompress
global huffDecompress

%macro begin 0
	push rbx
	push rdx
	push r12
	push r14
	push r15
%endmacro

%macro end 0
	pop r15
	pop r14
	pop r12
	pop rdx
	pop rbx
%endmacro

;; u32 *huffCompress(u32 *dst, u8 *src, u8 *end, u32 *tbl)
huffCompress:
    mov     r10,rcx            ; table with huffman codes/lengths
    mov     r8b,-32            ; bits to go
    xor     r11,r11
    
.loop:
    mov     r11b,byte [rsi]    ; load byte
    mov     ecx, [r10+4*r11]   ; load huffman code/length
    add	    r8b,cl             ; add code legth into 'r8b'
    jl	    .nostore           ; enough space in the register?

    sub     cl,r8b             ; no, put in what fits in and adjust 'cl'
    sub     r8b,32
    shld    eax,ecx,cl
    add     cl,r8b
    bswap   eax
    mov     [rdi],eax
    add     rdi,4

.nostore:                      ; put the code into 'eax'
    shld    eax,ecx,cl
    
    add     rsi,1              ; advance source by one byte
    cmp     rsi,rdx            ; loop...
    jnz     .loop

    cmp     r8b,-32            ; extra bits to flush?
    jle    .noextra
    
    mov     cl,r8b
    neg     cl
    shl     eax,cl
    bswap   eax
    mov     [rdi],eax
    add     rdi,4

.noextra:
    mov     rax,rdi            ; how many dwords did we encode?
    retn

huffDecompress:
    begin
    
    mov     r11,rcx               ; shift table
    mov     r12,rcx
    add     r12,256               ; pointers to decoder tables
	
	mov     rax,0                 ; bit-position
	
.loop:
    mov     rbx,rax               ; copy bit-position
    mov     cl,al                 ; bit-position in the current qword
    shr     rbx,6                 ; bit-position / 64 (eg. index of current qword in src)
    mov     r15,[rsi+rbx*8]       ; load 1. dword
    mov     r14,[rsi+rbx*8+8]     ; load 2. dword
    bswap   r15
    bswap   r14
    shld    r15,r14,cl            ; left-shift to have the beginning on the very left position
    shr     r15,32
    or      r15,1
    shl     r15,32
    bsr     rbx,r15                ; store MSB in rbx
    btr     r15,rbx                ; clear MSB from r15
    sub     rbx,32
    mov     rbx,[r12+rbx*8]       ; load pointer to the decoder table
    mov     cl,[rbx]              ; first byte is residual symbol length
    add     cl,32
    shr     r15,cl                 ; right-shift to extract the symbol
    movzx   rcx,byte [rbx+r15+1]   ; load decoded byte
    movzx   rbx,byte [r11+rcx]    ; load symbol length
    add     rax,rbx               ; add symbol length to rax
    
    mov     [rdi],cl              ; store decoded symbol
    add	    rdi,1                 ; advance dst
    
    cmp     rdi,rdx
    jb      .loop
    
    end
    
    mov     rax,r10
    
    retn
