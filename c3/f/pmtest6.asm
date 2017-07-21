;
; nasm pmtest6.asm -o pmtest.bin
; 
;

%include        "pm.inc"

PageDirBase     equ     200000h     ; 页目录开始地址： 2M
PageTblBase     equ     201000h     ; 页表开始地址： 2M + 4k


    org     07c00h
        jmp LABEL_BEGIN

[SECTION .gdt]
; GDT
;                                段基址，          段界限, 属性     
LABEL_GDT:          Descriptor        0,                0,  0               ; 空描述符
LABEL_DESC_NORMAL:  Descriptor        0,           0ffffh,  DA_DRW          ; 非一致代码段
LABEL_DESC_PAGE_DIR:Descriptor  PageDirBase,         4095,  DA_DRW          ; Page Directory
LABEL_DESC_PAGE_TBL:Descriptor  PageTblBase,         1023,  DA_DRW|DA_LIMIT_4K  ; Page Tables
LABEL_DESC_CODE32:  Descriptor        0, SegCode32Len - 1,  DA_C + DA_32    ; 非一致代码段
LABEL_DESC_CODE16:  Descriptor        0,           0ffffh,  DA_C            ; 非一致代码段
LABEL_DESC_DATA:    Descriptor        0,      DataLen - 1,  DA_DRW          ; Data
LABEL_DESC_STACK:   Descriptor        0,       TopOfStack,  DA_DRWA + DA_32 ; Stack,32位
LABEL_DESC_VIDE0：  Descriptor  0B8000h，          0ffffh,  DA_DRW          ; 显存首地址
; GDT 结束

GdtLen      equ     $-LABEL_GDT                                             ; GDT长度
GdtPtr      dw      GdtLen - 1                                              ; GDT界限
            dd      0                                                       ; GDT基地址   


; GDT选择子
SelectorNormal      equ     LABEL_DESC_NORMAL   - LABEL_GDT
SelectorPageDir     equ     LABEL_DESC_PAGE_DIR - LABEL_GDT
SelectorPageTbl     equ     LABEL_DESC_PAGE_TBL - LABEL_GDT
SelectorCode32      equ     LABEL_DESC_CODE32   - LABEL_GDT
SelectorCode16      equ     LABEL_DESC_CODE16   - LABEL_GDT
SelectorData        equ     LABEL_DESC_DATA     - LABEL_GDT
SelectorStack       equ     LABEL_DESC_STACK    - LABEL_GDT
SelectorVideo       equ     LABEL_DESC_VIDE0    - LABEL_GDT


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

LABEL_SEG_CODE32:
        call SetupPaging

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

        call TestRead
        call TestWrite
        call TestRead

        jmp SelectorCode16:0

;------------------------------------------------------------------------
; 启动分页机制
SetupPaging:
        ; 为了简单处理，所有线性地址对应相等的物理地址

        ; 初始化页目录
        mov ax,SelectorPageDir
        mov es,ax
        mov ecx,1024
        xor edi,edi
        xor eax,eax
        mov eax,PageTblBase | PG_P | PG_USU | PG_RWW
    .1:
        stosd
        add eax,4096
        loop .1

        ; 再初始化所有页表
        mov ax,SelectorPageTbl
        mov es,ax
        mov ecx,1024 * 1024
        xor edi,edi
        xor eax,eax
        mov eax,PG_P|PG_USU|PG_RWW
    .2: 
        stosd
        add eax,4096
        loop .2
        
        mov eax,PageDirBase
        mov cr3,eax 
        mov eax,cr0
        or eax,80000000h
        mov cr0,eax
        jmp short .3
    .3:
        nop
        ret


SegCode32Len    equ $ - LABEL_SEG_CODE32


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
        and eax,7FFFFFFEh
        mov cr0,eax

LABEL_GO_BACK_TO_REAL:
        jmp 0:LABEL_REAL_ENTRY

Code16Len       equ $ - LABEL_SEG_CODE16
