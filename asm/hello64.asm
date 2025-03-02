bits 64

; https://p403n1x87.github.io/getting-started-with-x86-64-assembly-on-linux.html
;
;   We want to export the `_start` symbol, as it is the entry point.
global _start

;
; CONSTANTS
;
SYS_WRITE   equ 1
SYS_EXIT    equ 60
STDOUT      equ 1

;
; Initialized data goes here
;
SECTION .data
hello           db "Hi mom!", 10    ; char *: 10 is the ASCII code for '\n'.
hello_len       equ $ - hello       ; size_t: subtract current address ($) from 'hello'

;
; Code goes here.
;
SECTION .text

_start:
    ; order of registers: (rdi, rsi, rdx, r10, r8, r9)
    ;   `syscall` requires the desired call to be in `rax`.
    ;   After a `syscall`, the return value is stored in `rax`.

    ; syscall(SYS_WRITE, fd=STDOUT, buf=hello, count=hello_len);
    mov     rax, SYS_WRITE
    mov     rdi, STDOUT
    mov     rsi, hello
    mov     rdx, hello_len
    syscall
    push    rax ; The return value of `SYS_WRITE` is now on top of the stack.

    ; syscall(SYS_EXIT, <sys_write return value> - hello_len);
    mov     rax, SYS_EXIT
    pop     rdi ; Pop the top of the stack but save it into `rdi`.
    sub     rdi, hello_len ; Determine if we printed all the characters.
    syscall
