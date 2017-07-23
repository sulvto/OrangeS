org 0100h

    jmp LABEL_START ;Strrt

; FAT12 磁盘的头
%include "fat12hdr.inc"
%include "load.inc"
%include "pm.inc"

; GDT

LABEL_GDT:          Descriptor 0,             0,  0           ;
LABEL_DESC_FLAT_C:  Descriptor 0,       0fffffh,DA_CR|DA_32|DA_LIMIT_4K ;0-4G
LABEL_DESC_FLAT_RM: Descriptor 0,       0fffffh,DA_DRM|DA_32|DA_LIMIT_4K;0-4G
LABEL_DESC_VIDEO:   Descriptor 0B8000h,  0ffffh,DA_DRM|DA_DPL3  ;显存首地址

GdtLen      equ     $ - LABEL_GDT
GdtPtr      dw      GdtLen - 1                      ; 段界线
            dd      BaseOfLoaderPhyAddr + LABEL_GDT ; 基地址

; GDT 选择子
SelectorFlatC       equ     LABEL_DESC_FLAT_C   -   LABEL_GDT
SelectorFlatRw      equ     LABEL_DESC_FLAT_RM  -   LABEL_GDT
SelectorVideo       equ     LABEL_DESC_VIDEO    -   LABEL_GDT + SA_RPL3

BaseOfStack     equ     0100h
PageDirBase     equ     100000h ; 页目录开始地址：1M 
PageTblBase     equ     101000h ; 页表开始地址：1M + 4K

LABEL_START:                ; Start
        mov ax,cs
        mov ds,ax
        mov es,ax
        mov ss,ax
        mov sp,BaseOfStack

        mov dh,0            ; "Loading "
        call DispStr
        
        ; 得到内存数
        mov ebx,0           ; 
        mov di,_MemChkBuf   ; es:di
.MemChkLoop:
        mov eax,0E820h      ;
        mov ecx,20
        mov edx,0534D4150h  ; edx = "SMAP"
        int 15h
        jc  .MemChkFail
        add di,20
        inc dword [_dwMCRNumber]    ;dwMCRNumber = ARDS 的个数
        cmp edx,0
        jne .MemChkLoop
        jmp .MemChkOk
.MemChkFail:
        mov dword [_dwMCRNumber],0
.MemChkOk:
        ; 在A盘的根目录寻找KERNEL.BIN
        mov word [wSectorNo],SectorNoOfRootDirectory
        xor ah,ah
        xor dl,dl
        int 13h

LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
        cmp word [wRootDirSizeForLoop],0
        jz LABEL_NO_KERNELBIN
        dec word [wRootDirSizeForLoop]
        mov ax,BaseOfKernelFile
        mov es,ax
        mov bx,OffsetOfKernelFile
        mov ax,[wSectorNo]
        mov cl,1
        call ReadSector

        mov si,KernelFileName
        mov di,OffsetOfKernelFile
        cld
        mov dx,10h
        
LABEL_SEARCH_FOR_KARNELBIN:
        cmp dx,0
        jz LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR
        dec dx
        mov cx,11

LABEL_CMP_FILENAME:
        cmp cx,0
        jz LABEL_FILENAME_FOUND
        dec cx
        lodsb
        cmp al,byte [es:di]
        jz LABEL_GO_ON
        jmp LABEL_DIFFERENT

LABEL_GO_ON:
        inc di
        jmp LABEL_CMP_FILENAME

LABEL_DIFFERENT:
        and di,0FFE0h
        add di,20h
        mov si,KernelFileName
        jmp LABEL_SEARCH_FOR_KARNELBIN

LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR:
        add word [wSectorNo],1
        jmp LABEL_SEARCH_IN_ROOT_DIR_BEGIN

LABEL_NO_KERNELBIN:
        mov dh,2
        call DispStr
        jmp $

LABEL_FILENAME_FOUND:
        mov ax,RootDirSectors
        and di,0FFF0h           ; 当前条目的开始
        
        push eax
        mov eax,[es : di + 01Ch]
        mov dword [dwKernelSize],eax
        pop eax
        add di,01Ah
        mov cx,word [es:di]
        push cx
        add cx,ax
        add cx,DeltaSectorNo
        mov ax,BaseOfKernelFile
        mov es,ax
        mov bx,OffsetOfKernelFile
        mov ax,cx

LABEL_GOON_LOADING_FILE:
        push ax
        push bx
        mov ah,0Eh
        mov al,'.'
        mov bl,0Fh
        int 10h
        pop bx
        pop ax

        mov cl,1
        call ReadSector
        pop ax
        call GetFATEntry
        cmp ax,0FFFh
        jz LABEL_FILE_LOADED
        push ax
        mov dx,RootDirSectors
        add ax,dx
        add ax,DeltaSectorNo
        add bx,[BPB_BytePerSec]
        jmp LABEL_GOON_LOADING_FILE

LABEL_FILE_LOADED:
        call KillMotor
        mov dh,1
        call DispStrRealMode
        
        ; 准备跳入保护模式
        
        ; 加载 GDTR
        lgdt    [GdtPtr]

        ; 关中断
        cli
        
        ; 打开地址线A20
        in  al,92h
        or  al,00000010b
        out 92h,al

        ; 准备切换到保护模式
        mov eax,cr0
        or  eax,1
        mov cr0,eax

        ; 进入保护模式
        jmp dword SelectorFlatC:(BaseOfLoaderPhyAddr+LABEL_PM_START)


;=============================================================
wRootDirSizeForLoop dw RootDirSectors
wSectorNo           dw 0                ; 要读的扇区号
b0dd                db 0                ; 
dwKernelSize        dd 0
;------------------------------------------------------------
KernelFileName      db  "KERNEL  BIN",0
MessageLength       equ 9
LoadMessage:        db  "[Loading]"
Message1            db  "Ready.   "
Message2            db  "No Kernel"
;=============================================================
DispStr:      ; 显示字符串
        mov ax,MessageLength
        mul dh
        add ax,LoadMessage
        mov bp,ax
        mov ax,ds
        mov es,ax
        mov cx,MessageLength
        mov ax,01301h
        mov bx,0007h
        mov dl,0
        int 10h
        ret

ReadSector:
        push    bp
        mov bp, sp
        sub esp,2
        mov byte [bp-2],cl
        push bx
        mov bl,[BPB_SecPerTrk]
        div bl
        inc ah
        mov cl,ah
        mov dh,al
        shr al,1
        mov ch,al
        and dh,1
        pop bx
        
        mov dl,[BS_DrvNum]
.GoOnReading:
        mov ah,2
        mov al,byte [bp-2]
        int 13h
        JC .GoOnReading
        
        add esp,2
        pop bp
        ret

; 找到序号为 ax 的 Sector 在 FAT 中的条目，结果放在 ax 中
GetFATEntry:
        push es
        push bx
        push ax
        mov ax,BaseOfKernelFile
        sub ax,0100h
        mov es,ax
        pop ax
        mov byte [b0dd],0
        mov bx,3
        mul bx
        mov bx,2
        div bx
        cmp dx,0
        jz LABEL_EVEN
        mov byte [b0dd],1
LABEL_EVEN:
        xor dx,dx
        mov bx,[BPB_BytePerSec]
        div bx
        push dx
        mov bx,0
        add ax,SectorNoOfFAT1
        mov cl,2
        call ReadSector

        pop dx
        add bx,dx
        mov ax,[es:bx]
        cmp byte [b0dd],1
        jnz LABEL_EVEN_2
        shr ax,4
LABEL_EVEN_2:
        and ax,0FFFh

KillMotor:
        push dx
        mov dx,03F2h
        mov al,0
        out dx,al
        pop dx
        ret


; 保护模式下执行==============================
[SECTION .s32]

ALIGN   32

[BITS   32]

LABEL_PM_START:
    mov ax,SelectorVideo
    mov gs,ax
    mov ax,SelectorFlatRw
    mov ds,ax
    mov es,ax
    mov fs,ax
    mov ss,ax
    mov esp,TopOfStack

    push szMemChkTitle
    call DispStr
    add  esp,4

;;    call DispMemInfo
    call SetupPaging

    mov ah,0Fh
    mov al,'P'
    mov [gs:((80 * 0 + 39) * 2)],ax
    jmp $

; %include    "lib.inc"

DispMemInfo:
    push    esi
    push    edi
    push    ecx
    
    mov esi,MemChkBuf
    mov ecx,[dwMCRNumber]
.loop:
    mov edx,5
    mov edi,ARDStruct
.1:
    push    dword [esi]
    call    DispInt
    pop     eax
    stosd
    add     esi,4
    dec     edx
    cmp     edx,0
    jnz     .1
    call    DispReturn
    cmp     dword [dwType],1
    jne     .2
    mov     eax, [dwBaseAddrLow]
    add     eax, [dwLengthLow]    
    cmp     eax, [dwMemSize]
    jb      .2
    mov     [dwMemSize],eax
.2:
    loop    .loop
    call    DispReturn
    push    szRAMSize
    call    DispStr
    add     esp,4

    pop     ecx
    pop     edi
    pop     esi
    ret
;; TODO DispReturn,DispInt

; 启动分页机制
SetupPaging:
    xor     dex,edx
    mov     eax,[dwMemSize]
    mov     ebx,400000h         ; 400000h = 4M = 4096 * 1024
    div     ebx
    mov     ecx,eax
    test    edx,edx
    jz      .no_remainder
    inc     ecx
.no_remainder:
    push    ecx

    ; 为简化处理，所有线性地址对应相等的物理地址

    ; 首先初始化页目录
    mov     ax,SelectorFlatRw
    mov     es,ax
    mov     edi,PageDirBase         ; 此段首地址为 PageDirBase
    xor     eax,eax
    mov     eax,PageTblBase | PG_P | PG_USU | PG_RWW
.1:
    stosd
    add     eax,4096                ; 为了简化，所有也表在内存中都是连续地
    loop    .1
            
    ; 再初始化所有页表
    pop     eax                     ; 页表个数
    mov     ebx,1024                ; 每个页表 1024 个PTE
    mul     ebx
    mov     ecx,eax
    mov     edi,PageTblBase         ; 此段首地址为 PageTblBase
    xor     eax,eax
    mov     eax,PG_P | PG_USU | PG_RWW
.2:
    stosd
    add     eax,4096                ; 每一页指向 4K 的空间
    loop    .2
    mov     eax,PageDirBase
    mov     cr3,eax
    mov     eax,cr0
    or      eax,80000000h
    mov     cr0,eax
    jmp     short .3
.3:
    nop
    
    ret

[SECTION .daata1]

ALIGN   32

LABEL_DATA:
; 实模式下使用
; 字符串
_szMemChkTitle: db "BaseAddrL BaseAddrH LengthLow LengthHigh   Type", 0Ah, 0
_szRAMSize:     db "RAM size:",0
_szReturn:      db 0Ah,0
; 变量
_dwMCRNumber:   dd 0    ; Memory Check Result
_dwDispPos:     dd (80 * 6 + 0) * 2     ; 屏幕6行0列
_dwMemSize:     dd 0
_ARDStruct:     ; Address Range Descriptor Structure
    _dwBaseAddrLow:     dd      0
    _dwBaseAddrHigh:    dd      0
    _dwLengthLow:       dd      0
    _dwLengthHigh:      dd      0
    _dwType:            dd      0
_MemChkBuf: times  256  db      0
;
; 保护模式下使用
szMemChkTitle       equ     BaseOfLoaderPhyAddr + _szMemChkTitle
szRAMSize           equ     BaseOfLoaderPhyAddr + _szRAMSize
szReturn            equ     BaseOfLoaderPhyAddr + _szReturn
dwDispPos           equ     BaseOfLoaderPhyAddr + _dwDispPos
dwMemSize           equ     BaseOfLoaderPhyAddr + _dwMemSize
dwMCRNumber         equ     BaseOfLoaderPhyAddr + _dwMCRNumber
ARDStruct           equ     BaseOfLoaderPhyAddr + _ARDStruct
    dwBaseAddrLow   equ     BaseOfLoaderPhyAddr + _dwBaseAddrLow
    dwBaseAddrHigh  equ     BaseOfLoaderPhyAddr + _dwBaseAddrHigh
    dwLengthLow     equ     BaseOfLoaderPhyAddr + _dwLengthLow
    dwLengthHigh    equ     BaseOfLoaderPhyAddr + _dwLengthHigh
    dwType          equ     BaseOfLoaderPhyAddr + _dwType
MemChkBuf           equ     BaseOfLoaderPhyAddr + _MemChkBuf


StackSpace:     times   1024    db  0
TopOfStack      equ     BaseOfLoaderPhyAddr + $ ; 栈顶

