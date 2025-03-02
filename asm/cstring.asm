bits 64

;
; CONSTANTS
;
SYS_WRITE   equ 1
STDOUT      equ 1

;
; Code goes here.
;
section .text

;
; size_t cstring_len(const char *cstring)
;       [in]    rdi (const char *)
;       [out]   rax (size_t)
;
global cstring_len
cstring_len:
    ; Function prologue.
    push    rbp
    mov     rbp, rsp

    ; size_t len = 0;
    xor     rax, rax

    .loop_start:
    ; while (*(cstring + len) != '\0')
    cmp     byte [rdi + rax], 0
    je      .end_loop

    ; { ++len; }
    inc     rax
    jmp     .loop_start

    ; return len;
    .end_loop:

    ; Function epilogue.
    mov     rsp, rbp
    pop     rbp
    ret
