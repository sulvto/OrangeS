; lib.inc
    
; 显示 AL 中的数字
; input :
;   [dwDispPos]:    -- pos of video ram
;   AL: -- src

DispAl: 
        push ecx
        push edx
        push edi

        mov edi,[dwDispPos]
        
        mov ah,0Fh      ; 0000b: 黑底    1111b:白字
        mov dl,al
        shr al,4
        mov ecx,2
    .begin:
        and al,01111b
        cmp al,9
        ja .1
        add al,'0'    
        jmp .2
    .1:
        sub al,0Ah
        add al,'A'
    .2:
        mov [gs:edi],ax
        add edi,2
        mov al,dl
        loop .begin

        mov [dwDispPos],edi

        pop edi
        pop edx
        pop ecx

        ret

;---------------------------------------------------------------------

; 显示一个整形数
; input:
;       [esp + 4]: -- 32 bits int to display
;       [dwDispPos]: -- pos of video ram
DispInt:
        mov eax,[esp+4]
        shr eax,24      ; byte 3 of int32
        call DispAl

        mov eax,[esp+4]
        shr eax,16      ; byte 2 of int32
        call DispAl

         mov eax,[esp+4]                         
         shr eax,8      ; byte 1 of int32
         call DispAl

        mov eax,[esp+4] ; byte 0 of int32
        call DispAl

        mov ah,07h
        mov al,'h'
        push edi
        mov edi,[dwDispPos]
        mov [gs:edi],ax
        add edi,4
        mov [dwDispPos],edi
        pop edi
            
        ret

;---------------------------------------------------------------------

; 显示一个字符串
; input:
;       [esp+4]: -- 32 bits int to display
;       [dwDispPos]: -- pos o video ram
;

DispStr:
        push ebp
        push ebx
        pueh esi
        push edi

        mov ebp,esp
        mov esi [ebp+8] ; pszInfo
        mov edi,[dwDispPos] 
        mov ah,0Fh
    .1:
        lodsb
        test al,al
        jz .2
        cmp al,oAh
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
        mov [eg:edi],ax
        add edi,2
        jmp .1
    .2:
        mov [dwDispPos],edi
        
        pop edi
        pop esi
        pop ebx
        pop ebp
        ret

;---------------------------------------------------------------------

;; 换行
DiepReturn:
        push szReturn
        call DispStr
        add esp,4
        ret
