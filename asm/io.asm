; https://userpages.cs.umbc.edu/chang/cs313.f04/nasmdoc/html/nasmdoc5.html
bits 64

;
; CONSTANTS
;   See: https://chromium.googlesource.com/chromiumos/docs/+/master/constants/syscalls.md
;
SYS_READ    equ 0
SYS_WRITE   equ 1


section .text

;
;   ssize_t read(int fd, void *buf, size_t count)
;
global read
read:
    push    rbp
    mov     rbp, rsp

    ; read(rdi=fd, rsi=buf, rdx=count)
    mov     rax, SYS_READ
    syscall

    mov     rsp, rbp
    pop     rbp
    ret

;
;   ssize_t write(int fd, const void *buf, size_count)
;
global write
write:
    push    rbp
    mov     rbp, rsp

    mov     rax, SYS_WRITE
    syscall

    mov     rsp, rbp
    pop     rbp
    ret
