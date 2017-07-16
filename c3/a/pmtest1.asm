;
; nasm pmtest1.asm -o pmtest1.bin
; 
;

%include        "pm.inc"

    org     07c00h
        jmp LABEL_BEGIN

[SECTION .gdt]
; GDT
LABEL_GDT:          Descriptor        0,                0,  0               ; 空描述符
LABEL_DESC_CODE32:  Descriptor        0, SegCode32Len - 1,  DA_C + DA_32    ; 非一致代码段
LABEL_DESC_VIDE0：  Descriptor  0B8000h，          0ffffh,  DA__DRW         ; 显存首地址
; GDT 结束

GdtLen      equ     $-LABEL_GDT                                             ; GDT长度
GdtPtr      dw      GdtLen - 1                                              ; GDT界限
            dd      0                                                       ; GDT基地址                    

; GDT选择子
SelectorCode32      equ     LABEL_DESC_CODE32   - LABEL_GDT
SelectorVideo       equ     LABEL_DESC_VIDE0    - LABEL_GDT


[SECTION .s16]
[bits 16]
LABEL_BEGIN:
        mov ax,cs
        mov ds,ax
        mov es,ax
        mov ss,ax
        mov sp,0100h

        ; 初始化 32 位代码段描述符
        xor eax,eax
        mov ax,cs
        shl eax,4
        add eax,LABEL_SEG_CODE32
        mov word [LABEL_SEG_CODE32 + 2],ax
        shr eax,16
        mov byte [LABEL_DESC_CODE32 + 4],al
        mov byte [LABEL_DESC_CODE32 + 7],ah

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
        

[SECTION .s32]

LABEL_SEG_CODE32:
        mov ax,SelectorVideo
        mov gs,ax
        
        mov edi,(80 * 11 + 79) * 2
        mov ah,0ch
        mov al,'p'
        mov [gs:di],ax

        jmp $

SegCode32Len    equ $-LABEL_SEG_CODE32







