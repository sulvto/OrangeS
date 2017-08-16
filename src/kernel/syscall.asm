%include "sconst.inc"

INT_VECTOR_SYS_CSLL equ 0x90
_NR_get_ticks       equ 0
_NR_write           equ 1
_NR_sendrec         equ 2

global get_ticks
global write

bits 32

[section .text]

get_ticks:
        mov eax,_NR_get_ticks
        int INT_VECTOR_SYS_CSLL
        ret

write:
        mov eax,_NR_write
        mov ebx,[esp + 4]
        mov ecx,[esp + 8]
        int INT_VECTOR_SYS_CSLL
        ret

sendrec:
        mov eax,_NR_sendrec
        mov ebx,[esp + 4]   ; function
        mov ecx,[esp + 8]   ; src_dest
        mov edx,[esp + 12]  ; p_img
        int INT_VECTOR_SYS_CSLL
        ret
