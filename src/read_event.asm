section .text
global read_event

read_event:
    mov rax, 0
    syscall
    ret
