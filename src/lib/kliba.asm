%include "sconst.inc"

extern disp_pos    

[section .text]

global disp_str
global disp_color_str
global out_byte
global in_byte
global enable_irq
global disable_irq
global disable_int
global enable_int


; 显示一个字符串
disp_str:
        push ebp
        mov ebp,esp

        mov esi,[ebp + 8]
        mov edi,[disp_pos]
        mov ah,0Fh
    .1:
        lodsb
        test al,al
        jz .2
        cmp al,0Ah          ; 回车？
        jnz .3
        push eax
        mov eax,edi
        mov bl,160
        div bl
        and eax,0FFh
        inc eax
        mov bl,160
        mul bl
        mov edi,eax
        pop eax
        jmp .1
    .3:
        mov [gs:edi],ax
        add edi,2
        jmp .1
    .2:
        mov [disp_pos],edi

        pop ebp
        ret
;---------------------------------------------------------------------
; void disp_color_str(char * info, int color);
disp_color_str:
        push ebp
        mov ebp,esp
        
        mov esi,[ebp + 8]   ; pszInfo
        mov edi,[disp_pos]
        mov ah,[ebp + 12];  ; color
    .1:
        lodsb
        test al,al
        jz .2
        cmp al,0Ah          ; 回车?
        jnz .3
        push eax
        mov eax,edi
        mov bl,160
        div bl
        and eax,0FFh
        inc eax
        mov bl,160
        mul bl
        mov edi,eax
        pop eax
        jmp .1
    .3:
        mov [gs:edi],ax
        add edi,2
        jmp .1

    .2:
        mov [disp_pos],edi

        pop ebp
        ret

;---------------------------------------------------------------------
; void out_byte(u16 port,u8 value)
out_byte:
        mov edx,[esp + 4]       ; port
        mov al,[esp + 4 + 4]    ; value
        out dx,al
        nop
        nop
        ret
;---------------------------------------------------------------------
; u8 in_byte(u16 port)
in_byte:
        mov edx,[esp + 4]       ; port
        xor eax,eax
        in al,dx
        nop
        nop
        ret

;---------------------------------------------------------------------
; void disable_irq(int irq);
;
disable_irq:
        mov ecx,[esp + 4]       ; irq
        pushf
        cli
        mov ah,1
        rol ah,cl
        cmp cl,8
        jae disable_8
disable_0:
        in al,INT_M_CTLMASK
        test al,ah
        jnz dis_already
        or al,ah
        out INT_M_CTLMASK,al
        popf
        mov eax,1
        ret
disable_8:
        in al,INT_S_CTLMASK
        test al,ah
        jnz dis_already
        or al,ah
        out INT_S_CTLMASK,al
        popf
        mov eax,1
        ret
dis_already:
        popf
        xor eax,eax
        ret

;-----------------------------------------------------------------
; void enable_irq(int irq);
;
enable_irq:
        mov ecx,[esp + 4]
        pushf
        cli 
        mov ah,~1
        rol ah,cl
        cmp cl,8
        jae enable_8
enable_0:
        in al,INT_M_CTLMASK
        and al,ah
        out INT_M_CTLMASK,al
        popf
        ret
enable_8:
        in al,INT_S_CTLMASK
        and al,ah
        out INT_S_CTLMASK,al
        popf
        ret


;-----------------------------------------------------------------
; void disable_int();
;
disable_int:
        cli
        ret

;-----------------------------------------------------------------
; void enable_int();
;
enable_int:
        sti
        ret
