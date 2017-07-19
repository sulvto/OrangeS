;
; nasm pmtest3.asm -o pmtest.bin
; 
;

%include        "pm.inc"

    org     07c00h
        jmp LABEL_BEGIN

[SECTION .gdt]
; GDT
;                                段基址，          段界限, 属性     
LABEL_GDT:           Descriptor        0,                0,  0               ; 空描述符
LABEL_DESC_NORMAL:   Descriptor        0,           0ffffh,  DA_DRW          ; 非一致代码段
LABEL_DESC_CODE32:   Descriptor        0, SegCode32Len - 1,  DA_C + DA_32    ; 非一致代码段
LABEL_DESC_CODE16:   Descriptor        0,           0ffffh,  DA_C            ; 非一致代码段
LABEL_DESC_CODE_DEST:Descriptor        0,SegCodeDestLen -1,  DA_C + DA_32    ; 非一致代码段
LABEL_DESC_DATA:     Descriptor        0,      DataLen - 1,  DA_DRW + DA_DPL1; Data
LABEL_DESC_STACK:    Descriptor        0,       TopOfStack,  DA_DRWA + DA_32 ; Stack,32位
LABEL_DESC_VIDE0：   Descriptor  0B8000h，          0ffffh,  DA_DRW          ; 显存首地址
LABEL_DESC_LDT:      Descriptor        0,         LDTLen-1,  DA_LDT          ; LDT

; 门                        目标选择子          偏移      DCount             属性
LABEL_CALL_GATE_TEST: Gate SelectorCodeDest,         0,            0,  DA_386CGate+DA_DPL0

; GDT 结束


GdtLen      equ     $-LABEL_GDT                                             ; GDT长度
GdtPtr      dw      GdtLen - 1                                              ; GDT界限
            dd      0                                                       ; GDT基地址   


; GDT选择子
SelectorNormal       equ     LABEL_DESC_NORMAL      - LABEL_GDT
SelectorCode32       equ     LABEL_DESC_CODE32      - LABEL_GDT
SelectorCode16       equ     LABEL_DESC_CODE16      - LABEL_GDT
SelectorCodeDest     equ     LABEL_DESC_CODE_DEST   - LABEL_GDT
SelectorData         equ     LABEL_DESC_DATA        - LABEL_GDT
SelectorStack        equ     LABEL_DESC_STACK       - LABEL_GDT
SelectorVideo        equ     LABEL_DESC_VIDE0       - LABEL_GDT
SelectorLDT          equ     LABEL_DESC_LDT         - LABEL_GDT

SelectorCodeGateTest equ     LABEL_CALL_GATE_TEST   - LABEL_GDT


; 数据段
[SECTION .data1]
ALIGN   32
    [bits 32]
        LABEL_DATA:
        SPValueInRealMode   dw  0
        ; 字符串
        PMMessage:          db  "In Protect Mode now ^-^",0                 ; 在保护模式中显示    
        OffestPMMessage     equ PMMessage - $$
        StrTest:            db  "ABCDEFGHIJKMNOPQRSTUVWXYZ",0
        OffestStrTest       equ StrTest - $$
        DataLen             equ $ - LABEL_DATA


[SECTION .gs]
ALIGN   32
    [bits 32]
LABEL_STACK:
        times12 db 0
TopOfStack      equ  $ - LABEL_STACK - 1




[SECTION .s16]
[bits 16]
LABEL_BEGIN:
        mov ax,cs
        mov ds,ax
        mov es,ax
        mov ss,ax
        mov sp,0100h

        mov [LABEL_GO_BACK_TO_REAL+3],ax
        mov [SPValueInRealMode],sp

        ; 初始化 16 位代码段描述符
        mov ax,cs
        movzx eax,ax
        shl eax,4
        add eax,LABEL_SEG_CODE16
        mov word [LABEL_DESC_CODE16 + 2],ax
        shr eax,16
        mov byte [LABEL_DESC_CODE16 + 4],al
        mov byte [LABEL_DESC_CODE16 + 7],ah


        ; 初始化 32 位代码段描述符
        xor eax,eax
        mov ax,cs
        shl eax,4
        add eax,LABEL_SEG_CODE32
        mov word [LABEL_DESC_CODE32 + 2],ax
        shr eax,16
        mov byte [LABEL_DESC_CODE32 + 4],al
        mov byte [LABEL_DESC_CODE32 + 7],ah


        ; 初始化测试调用门的代码段描述符
        xor eax,eax
        mov ax,cs
        shl eax,4
        add eax,LABEL_SEG_CODE_DEAT
        mov word [LABEL_DESC_CODE_DEST + 2],ax
        shr eax,16
        mov byte [LABEL_DESC_CODE_DEST + 4],al
        mov byte [LABEL_DESC_CODE_DEST + 7],ah


        ; 初始化数据段描述符
        xor eax,eax
        mov ax,ds
        shl eax,4
        add eax,LABEL_DATA
        mov word [LABEL_DESC_DATA + 2],ax
        shr eax,16
        mov byte [LABEL_DESC_DATA + 4],al
        mov byte [LABEL_DESC_DATA + 7],ah

        ; 初始化堆栈段描述符
        xor eax,eax
        mov ax,ds
        shl eax,4
        add eax,LABEL_STACK
        mov word [LABEL_DESC_STACK + 2],ax
        shr eax,16
        mov byte [LABEL_DESC_STACK + 4],al
        mov byte [LABEL_DESC_STACK + 7],ah

        ; 初始化LDT在GDT中的描述符
        xor eax,eax
        mov ax,ds
        shl eax,4
        add eax,LABEL_LDT
        mov word [LABEL_DESC_LDT + 2],ax
        shr eax,16
        mov byte [LABEL_DESC_LDT + 4],al
        mov byte [LABEL_DESC_LDT + 7],ah

        ; 初始化LDT中的描述符
        xor eax,eax
        mov ax,ds
        shl eax,4
        add eax,LABEL_CODE_A
        mov word [LABEL_LDT_DESC_CODEA + 2],ax
        shr eax,16
        mov byte [LABEL_LDT_DESC_CODEA + 4],al
        mov byte [LABEL_LDT_DESC_CODEA + 7],ah

        ; 为加载 GDTR 作准备
        xor eax,eax
        mov ax,ds
        shl eax,4
        add eax,LABEL_GDT
        mov dword [GdtPtr + 2],eax

        ; 加载 GDTR
        lgdt    [GdtPtr]
        
        ; 关中断
        cli

        in al,92h
        or al,00000010b
        out 92h,al

        mov eax,cr0
        or  eax,1
        mov cr0,eax

        jmp dword SelectorCode32:0
        

;---------------------------------------------------------------------
; 从保护模式跳回到实模式
LABEL_REAL_ENTRY:
        mov ax,cs
        mov ds,ax
        mov es,ax
        mov ss,ax
        
        mov sp,[SPValueInRealMode]

        in al,92h
        and al,11111101b
        out 92h,al

        sti

        mov ax,4c00h
        int 21h


[SECTION .s32]
        [bits 32]
LABEL_SEG_CODE32:
        mov ax,SelectorData                 ; 数据段选择子
        mov ds,ax
        mov ax,SelectorVideo                ; 视频段选择子
        mov gs,ax

        mov ax,SelectorStack                ; 堆栈段选择子
        mov ss,ax

        mov esp,TopOfStack

        ; 显示一个字符串
        mov ah,0CH
        xor esi,esi
        xor edi,edi
        mov esi,OffestPMMessage
        mov edi,(80 * 10 + 0) * 2
        cld
    .1:
        loadsb
        test al,al
        jz .2
        mov [gs:edi],ax
        add edi,2
        jmp .1
    ; 显示完毕
    .2:
        call DispReturn

        ; 测试调用门
        call SelectorCodeGateTest:0

        ; load LDT
        mov ax,SelectorLDT
        lldt ax

        jmp SelectorLDTCodeA:0

;-----------------------------------------------------------------------

DispReturn:
        push eax
        push    ebx
        mov eax,edi
        mov bl,160
        div bl
        and eax,0FFh
        inc eax
        mov bl,160
        mul bl
        mov edi,eax
        pop ebx
        pop eax
        ret


SegCode32Len    equ $ - LABEL_SEG_CODE32


; 调用门目标段
[SECTION .sdest]
        [bits 32]

LABEL_SEG_CODE_DEST:
        mov ax,SelectorVideo
        mov gs,ax
        mov edi,(80*12+0)*2
        mov ah,0ch
        mov al,'c'
        mov [gs:edi],ax        
        retf

SegCodeDestLen  equ $ -LABEL_SEG_CODE_DEST 


[SECTION .s16code]
ALIGN 32
    [bits 16]
LABEL_SEG_CODE16:
        ; 跳回实模式
        mov ax,SelectorNormal
        mov ds,ax
        mov es,ax
        mov fs,ax
        mov gs,ax
        mov ss,ax

        mov eax,cr0
        and al,11111110b
        mov cr0,eax

LABEL_GO_BACK_TO_REAL:
        jmp 0:LABEL_REAL_ENTRY

Code16Len       equ $ - LABEL_SEG_CODE16


[SECTION .ldt]
ALIGN 32
LABEL_LDT:
;
LABEL_LDT_DESC_CODEA: Descriptor        0,        CodeALen-1,  DA_C + DA_32      ; Code 32

LDTLen      equ     $ - LABEL_LDT

; LDT 选择子
SelectorLDTCodeA    equ     LABEL_LDT_DESC_CODEA        - LABEL_LDT + SA_TIL

[SECTION .la]
ALIGN 32
    [bits 32]
LABEL_CODE_A:
        mov ax,SelectorVideo
        mov gs,ax

        mov edi,(80 * 12 + 0) * 2
        mov ah,0CH
        mov al,'L'
        mov [gs:edi],ax

        ; 跳回实模式
        jmp SelectorCode16:0

CodeALen        equ     $ - LABEL_CODE_A
