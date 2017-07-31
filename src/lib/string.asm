
[section .text]

; 导出函数
global memcpy
global memset

; 内存拷贝
memcpy:
        push ebp
        mov ebp,esp
        
        push esi
        push edi
        push ecx

        mov edi,[ebp + 8]
        mov esi,[ebp + 12]
        mov ecx,[ebp + 16]
    .1:
        cmp ecx,0
        jz .2
        
        mov al,[ds:esi]
        inc esi
        
        mov byte [es:edi],al
        inc edi
        
        dec ecx
        jmp .1
    .2:
        mov eax,[ebp + 8]
        pop ecx
        pop edi
        pop esi
        mov esp,ebp
        pop ebp

        ret
;---------------------------------------------------------------------
;
; void memset(void *p_dst, char ch, int size);
;
memset:
        push ebp
        mov ebp,esp
        
        push esi
        push edi
        push ecx

        mov edi,[ebp + 8]   ; Desctination
        mov edx,[ebp + 12]  ; Char to be putted
        mov ecx,[ebp + 16]  ; Counter
    .1:
        cmp ecx,0   
        jz .2

        mov byte [edi],dl
        inc edi

        dec ecx
        jmp .1
    .2:
        pop ecx
        pop edi
        pop esi
        mov esp,ebp
        pop ebp
        
        ret
