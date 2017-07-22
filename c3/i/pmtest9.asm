;
; nasm pmtest9.asm -o pmtest.bin
; 
;

%include        "pm.inc"

PageDirBase0     equ     200000h     ; 页目录开始地址： 2M
PageTblBase0     equ     201000h     ; 页表开始地址：   2M + 4k
PageDirBase1     equ     210000h     ; 页目录开始地址： 2M + 64K
PageTblBase1     equ     211000h     ; 页表开始地址：   2M + 64k + 4k


LinearAddrDemo  equ     00401000h
ProcFoo         equ     00401000h
ProcBar         equ     00501000h
ProcPaginDemo   equ     00301000h 

    org     0100h
        jmp LABEL_BEGIN

[SECTION .gdt]
; GDT
;                                段基址，          段界限, 属性     
LABEL_GDT:          Descriptor        0,                0,  0               ; 空描述符
LABEL_DESC_NORMAL:  Descriptor        0,           0ffffh,  DA_DRW          ; 非一致代码段
LABEL_DESC_FLAT_C:  Descriptor        0,           0ffffh,  DA_C|DA_32|DA_LIMIT_4K      ; 0~4G
LABEL_DESC_FLAT_RW: Descriptor        0,           0ffffh,  DA_DRW|DA_LIMIT_4K          ; 0~4G
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
SelectorFlatC       equ     LABEL_DESC_FLAT_C   - LABEL_GDT
SelectorFlatRW      equ     LABEL_DESC_FLAT_RW  - LABEL_GDT
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
        ; 实模式下使用这些符号
        ; 字符串
        ; 在保护模式中显示   
        _szPMMessage:       db  "In Protect Mode now ^-^",0Ah,oAh,0
    
        ; 进入保护模式后显示此字符串
        _szMemChkTitle:     db  "BaseAddrL BaseAddrH LengthLow LngthHigh Type",Oah,0
        _szRAMSize          db  "RAM size:",0
        _szReturn           db  0Ah,0

        ; 变量 
        _wSPValueInRealMode:dw  0
        ; memory check result
        _dwMCRNumber:       dd  0
        ; 屏幕第6行，第0列
        _dwDispPos:         dd  (80*6+0)*2  
        _dwMemSize:         dd  0
        ; Address Range Descriptor structure
        _ARDStruct:
            _dwBaseAddrLow: dd  0
            _dwBaseAddrHige:dd  0
            _dwLengthLow:   dd  0
            _dwLengthHige:  dd  0
            _dwType:        dd  0
        _PageTableNumber    dd  0
        _SavedIDTR:         dd  0               ; 用于保存 IDTR
                            dd  0
        _SavedIMREG:        db  0               ; 中断屏蔽寄存器值
        _MemChkBuf:    times 256 db 0
        
        ; 保护模式下使用这些符号
        szPMMessage     equ _szPMMessage    - $$
        szMemChkTitle   equ _szMemChkTitle  - $$ 
        szRAMSize       equ _szRAMSize      - $$
        szReturn        equ _szReturn       - $$    
        dwDispPos       equ _dwDispPos      - $$
        dwMemSize       equ _dwMemSize      - $$
        dwMCRNumber     equ _dwMemSize      - $$
        ARDStruct       equ _ARDStruct      - $$
            dwBaseAddrLow       equ _dwBaseAddrLow     - $$  
            dwBaseAddrHige      equ _dwBaseAddrHige    - $$ 
            dwLengthLow         equ _dwLengthLow       - $$ 
            dwLengthHige        equ _dwLengthHige      - $$ 
            dwType              equ _dwType            - $$ 
       
        MemChkBuf       equ _MemChkBuf      - $$
        SavedIDTR       equ _SavedIDTR      - $$
        SavedIMREG      equ _SavedIMREG     - $$
        PageTableNumber equ _PageTableNumber- $$

        DateLen         equ $ - LABEL_DATA

; IDT
[SECTION .idt]
ALIGN   32
    [bits 32]
LABEL_IDT:
; 门
%rep 32
            Gate    SelectorCode32,SpuriousHandler,0,   DA_386IGate
%endrep
.020h:  Gate    SelectorCode32,ClockHandler,  0,  DA_386IGate
%rep 95
            Gate    SelectorCode32,SpuriousHandler,0,   DA_386IGate
%endrep
.080h:  Gate    SelectorCode32,UserIntHandler,0,  DA_386IGate

IdtLen      equ         $ - LABEL_IDT
IdtPtr      dw  IdtLen - 1
            dd  0

; 全局堆栈段
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

        ; 得到内存数
        mov ebx,0
        mov di,_MemChkBuf
    .loop:
        mov eax,0E820h
        mov ecx,20
        mov edx,0534D4150h
        int 15h
        jc LABEL_MEM_CHK_FAIL
        add di,20
        inc dword [_dwMCRNumber]
        cmp ebx,0
        jne .loop
        jmp LABEL_MEM_CHK_OK
    LABEL_MEM_CHK_FAIL:
        mov dword [_dwMCRNumber],0
    LABEL_MEM_CHK_OK:

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

        ; 为加载 IDTR 作准备
        xor eax,eax
        mov ax,ds
        shl eax,4
        add eax,LABEL_IDT
        mov dword [IdtPtr + 2],eax

        ; 保存 IDTR
        sidt    [_SavedIDTR]
        ; 保存中断屏蔽寄存器（IMREG）值
        in al,21h
        mov [_SavedIMREG],al

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
        
        mov sp,[_wSPValueInRealMode]

        lidt [_SavedIDTR]       ; 恢复 IDTR 的值
        mov al,[_SavedIMREG]    ;恢复中断屏蔽寄存器（IMREG）的值
        out 21h,al

        in al,92h
        and al,11111101b
        out 92h,al

        

        mov ax,4c00h
        int 21h


[SECTION .s32]

LABEL_SEG_CODE32:
        mov ax,SelectorData                 ; 数据段选择子
        mov ds,ax
        mov es,ax

        mov ax,SelectorVideo                ; 视频段选择子
        mov gs,ax

        mov ax,SelectorStack                ; 堆栈段选择子
        mov ss,ax

        mov esp,TopOfStack

        call Init8259A
        int 080h
    sti
        jmp $

        ; 显示一个字符串

        push szPMMessage
        call DispStr
        add esp,4
    
        push szMemChkTitle
        call DispStr
        add esp,4

        call DispMemSize                    ; 显示内存信息

        call PagingDemo                    ; 启动分页信息

        call SetRealmode8259A

        ; 到此为止
        jmp SelectorCode16:0

;---------------------------------------------------------------------

Init8259A:
        mov al,011h
        out 020h,al
        call io_delay

        out 0A0h,al
        call io_delay
        
        mov al,020h
        out 021h,al
        call io_delay

        mov al,028h
        out 0A1h,al
        call io_delay

        mov al,004h
        out 021h,al
        call io_delay

        mov al,002h
        out 0A1h,al
        call io_delay
                      
        mov al,001h
        out 021h,al
        call io_delay

        out 0A1h,al
        call io_delay
                      
        mov al,11111110b
        out 021h,al
        call io_delay

        mov al,11111111h
        out 0A1h,al
        call io_delay
                              
        ret

;---------------------------------------------------------------------

SetRealmode8259A:
        mov ax,SelectorData
        mov fs,ax

        mov al,017h
        out 020h,al
        call io_delay

        mov al,008h
        out 021h,al
        call io_delay

        mov al,001h
        out 021h,al
        call io_delay


        mov al,[fs:SavedIMREG]
        out 021h,al
        call io_delay
        
        ret

;---------------------------------------------------------------------

io_delay:
        nop 
        nop
        nop
        nop
        ret

;---------------------------------------------------------------------

_ClockHandler:
ClockHandler        equ     _ClockHandler - $$
        inc byte [gs:((80 * 0 + 70) * 2)]
        mov al,20h
        out 20h,al
        iretd

_UserIntHandler:
UserIntHandler      equ     _UserIntHandler  - $$
        mov ah,0Ch
        mov al,'I'
        mov [gs:((80 * 0 + 70) * 2)],ax
        iretd

_SpuriousHandler:
SpuriousHandler     equ     _SpuriousHandler - $$
        mov ah,0Ch
        mov al,'!'
        mov [gs:((80 * 0 + 75) * 2)],ax
        jmp $
        iretd
;---------------------------------------------------------------------

;------------------------------------------------------------------------
; 启动分页机制
SetupPaging:
        ; 根据内存大小计算应初始化多少PDE及多少页表
        xor edx,edx
        mov eax,[dwMemSize]
        mov ebx,400000h                 ; 400000h = 4M = 4096 * 1024
        div ebx
        mov ecx,eax                     ; ecx为页表的个数，PDE应该的个数
        test edx,edx
        jz .no_remainder
        inc ecx                         ; 如果余数不为0就增加一个页表
    .no_remainder:
        mov [PageTableNumber] ecx                        ; 页表个数

        ; 为了简化处理，所有线性地址对应相等的物理地址，并且不考虑内存空洞

        ; 初始化页目录
        mov ax,SelectorFlatRW
        mov es,ax
        mov edi,PageDirBase0
        xor eax,eax
        mov eax,PageTblBase0 | PG_P | PG_USU | PG_RWW
    .1:
        stosd
        add eax,4096
        loop .1

        ; 再初始化所有页表
        mov ax,[PageTableNumber]        ; 页表个数
        mov ebx,1024                    ; 每个页表1024个PTE
        mul ebx
        mov ecx,eax                     ; PTE个数 = 页表个数 * 1024
        xor edi,PageTblBase0
        xor eax,eax
        mov eax,PG_P|PG_USU|PG_RWW
    .2: 
        stosd
        add eax,4096                    ; 每一页指向 4K 的空间
        loop .2
        
        mov eax,PageDirBase0
        mov cr3,eax 
        mov eax,cr0
        or eax,80000000h
        mov cr0,eax     
        jmp short .3
    .3:
        nop
        ret

;---------------------------------------------------------------------


;
; 测试分页机制
;
PagingDemo:
        mov ax,cs
        mov ds,ax
        mov ax,SelectorFlatRW
        mov es,ax

        push LenFoo
        push OffsetFoo
        push ProcFoo
        call MemCpy
        add esp,12

        push LenBar
        push OffsetBar
        push ProcBar
        call MenCpy
        add esp,12

        push LenPagingDemoAll   
        push OffsetPagingDemoProc
        push ProcPagingDemo
        call MemCpy
        add esp,12

        mov ax,SelectorData
        mov dx,ax
        mov es,ax
        
        call SetupPaging            ; 启动分页

        call SelectorFlatC:ProcPagingDemo
        call PSwitch                ; 切换页目录，改变地址映射关系
        call SelectorFlatC:ProcPagingDemo

        ret

;---------------------------------------------------------------------

; 切换页表
PSwitch:
        mov ax,SelectorFlatRW
        mov es,ax
        mov edi,PageDirBase1
        xor eax,eax
        mov eax,PageTblBase1 | PG_P | PG_USU | PG_RWW
        mov ecx,[PageTableNumber]
    .1:
        stosd
        add eax,4096
        loop .1
        
        mov eax,[PageTableNumber]
        mov ebx,1024
        mul ebx
        mov ecx,eax
        mov edi,PageTblBase1
        xor eax,eax
        mov eax,PG_P | PG_USU | PG_RWW
    .2:
        srosd
        add eax,4096
        loop .2
        
        mov eax,LinearAddrDemo
        shr eax,22
        mov ebx,4096
        mul ebx
        mov ecx,eax
        mov eax,LinearAddrDemo
        shr eax,12
        and eax,03FFh
        mov ebx,4
        mul ebx
        add eax,ecx
        add eax,PageTblBase1

        mov dword [es:eax],ProcBar | PG_P | PG_USU | PG_RWW
        mov eax,PageDirBase1
        mov cr3,eax
        jmp short .3
    .3:
        nop
    
        ret
;---------------------------------------------------------------------

PagingDemoProc:
OffsetPagingDemoProc        equ     PagingDemoProc - $$
        mov eax,LinearAddrDemo
        call eax
        reft
LenPagingDemoAll        equ     4 - PagingDemoProc

foo:
OffsetFoo               equ     foo - $$
        mov ah,0Ch                      ;  0000：黑底    1100：红字
        mov al,'F'
        mov [gs:((80 * 17 + 0) * 2)],ax ; 屏幕第 17 行，第 0 列    
        mov al,'o'
        mov [gs:((80 * 17 + 1) * 2)],ax ; 屏幕第 17 行，第 1 列      
        mov [gs:((80 * 17 + 2) * 2)],ax ; 屏幕第 17 行，第 2 列    
        ret
LenFoo          equ     $ - foo

bar:
OffsetBar       equ     bar - $$
        mov ah,0Ch
        mov al,'B'
        mov [gs:((80 * 18 + 0) * 2)],ax ; 屏幕第 18 行，第 0 列      
        mov al,'a'
        mov [gs:((80 * 18 + 1) * 2)],ax ; 屏幕第 18 行，第 1 列      
        mov al,'r'
        mov [gs:((80 * 18 + 2) * 2)],ax ; 屏幕第 18 行，第 2 列      
LenBar      equ     $ - bar

;---------------------------------------------------------------------
; 显示内存信息
DispMemSize:
        push esi
        push edi    
        push ecx

        mov esi,MemChkBuf
        mov ecx,[dwMCRNumber]
    .loop:
        mov edx,5
        mov edi,ARDStruct
    .1:
        push dword [esi]        
        call DispInt
        pop eax
        stosd
        add esi,4
        dec edx
        cmp edx,0
        jnz .1
        call DispReturn
        
        cmp dword [dwType],1
        jne .2
        mov eax,[dwBaseAddrLow]
        add eax,[dwLengthLow]
        cmp eax,[dwMemSize]
        jb .2
        mov [dwMemSize],eax
    .2:
        loop .loop

        call DispReturn
        push szRAMSize
        call DispStr
        add esp,4

        push dword [dwMemSize]
        call DispInt
        add esp,4
        
        pop ecx
        pop edi
        pop esi
        ret

%include    "lib.inc"       

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
