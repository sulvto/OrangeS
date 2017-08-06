
[section .text]

; 导出函数
global memcpy
global memset
global strcpy

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

        mov edi,[ebp + 8]   ; Destination
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

;
; void strcpy(char* p_dst, char* p_src);
;
strcpy:
        push ebp
        mov ebp,esp
        
        mov esi,[ebp + 12]  ; src
        mov edi,[ebp + 8]   ; dst

    .1:
        mov al,[esi]
        inc esi

        mov byte [edi],al
        inc edi

        cmp al,0
        jnz .1
    
        mov eax,[ebp + 8]

        pop ebp
        ret
