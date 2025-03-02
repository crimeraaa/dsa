bits 64

;
; bool string_eq(String a, String b)
;   [in]
;       rdi = const char *a.data
;       rsi = size_t      a.len
;       rdx = const char *b.data
;       rcx = size_t      b.len
;   [out]
;       al  = bool equals
;
global string_eq
string_eq:
    push    rbp
    mov     rbp, rsp

    ; bool equals = false;
    xor     rax, rax

    ; // Fast path #1: strings of differing lengths are never equal.
    ; if (a.len != b.len) return equals = false;
    cmp     rsi, rcx
    jne     .comparison_end

    ; // Fast path #2: strings of the same length that are both 0 are always equal.
    ; else if ((a.len & b.len) == 0) return equals = true;
    test    rsi, rsi
    jz      .comparison_true

    ; // Fast path #3: strings of the same length and pointers are always equal.
    ; else if (a.data == b.data) return equals = true;
    cmp     rdi, rcx
    je      .comparison_true

    ; mem_compare(rdi = a.data, rsi = b.data, rdx = a.len)
    mov     rbx, rsi    ; rbx = a.len   // temporary register.
    mov     rsi, rdx    ; rsi = b.data
    mov     rdx, rbx    ; rdx = rbx = a.len
    call    mem_compare

    test    rax, rax
    jz      .comparison_true    ; if (rax == 0) return rax = true
    mov     rax, 0              ; else return rax = false
    jmp     .comparison_end

    .comparison_true:
    mov     rax, 1

    .comparison_end:
    mov     rsp, rbp
    pop     rbp
    ret

;
; int mem_compare(const void *s1, const void *s2, size_t n)
;
global mem_compare
mem_compare:
    push    rbp
    mov     rbp, rsp

    xor     rcx, rcx    ; size_t i = 0;

    .loop_start:
    cmp     rcx, rdx    ; i < n
    jge     .loop_end

    movsx   rax, byte [rdi + rcx] ; int b1 = (int)*((unsigned char *)s1 + i)
    movsx   rbx, byte [rsi + rcx] ; int b2 = (int)*((unsigned char *)s2 + i)
    sub     rax, rbx    ; int diff = b1 - b2;
    test    rax, rax
    jnz     .loop_end   ; if (diff != 0) return diff;

    .loop_increment:
    inc     rcx         ; ++i
    jmp     .loop_start

    .loop_end:
    mov     rsp, rbp
    pop     rbp
    ret
