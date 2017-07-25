; 显示 AL 中的数字
DispAl:
    ; TODO

; 显示一个整数型
DispInt:
    ; TODO

; 显示一个字符串
DispStr:
    ； TODO

; 换行
DispReturn:
        push szReturn
        call DispStr
        add esp,4

        ret

;---------------------------------------------------------------------

; 内存拷贝
MemCpy:
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
        jz .z
        
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
