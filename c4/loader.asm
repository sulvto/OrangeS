org 0100h
        mov ax,0B800h
        mov gs,ax
        mov ah,0Fh
        mov al,'L'
        mov [GS:((80 * 0 + 39) * 2)],ax

        jmp $
