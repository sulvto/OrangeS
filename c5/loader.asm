        org     0100h
        
        jmp start

; FAT12 磁盘的头
%include    "fat12hdr.inc"
%include    "load.inc"
%include    "pm.inc"

; GDT
;
GDT:            Descriptor      0,        0,  0           
DESC_FLA_C:     Descriptor      0,  0fffffh, DA_CR|DA_32|DA_LIMIT_4K
DESC_FALT_RW:   Descriptor      0,  0fffffh, DA_DRW|DA_32|DA_LIMIT_4K 
DESC_VISEO:     Descriptor  0B8000h, 0ffffh, DA_DRW|DA_DPL3

GdtLen      equ $ - GDT                     
GdtPtr      dw  GdtLen - 1                ; 段界限
            dd  BaseOfLoaderPhyAddr + GDT ; 基地址

; GDT 选择子
SelectorFlatC       equ     DESC_FLA_C      - GDT
SelectorFlatRW      equ     DESC_FALT_RW    - GDT
SelectorVideo       equ     DESC_VISEO      - GDT + SA_RPL3


        BaseOfStack         equ     0100h


start:
        mov ax,cs
        mov ds,ax
        mov es,ax
        mov ss,ax
        mov sp,BaseOfStack

        mov dh,0                           ; "Loading  "
        call DispStrRealMode

        ; 得到内存数
        mov ebx,0
        mov di,_MemChkBuf
    .MemChkLoop:
        mov eax,0E820h
        mov ecx,20
        mov edx,0534D4150h
        int 15h
        jc  .MemChkFail
        add di,20
        inc dword [_dwMCRNumber]
        cmp ebx,0
        jne .MemChkLoop
        jmp .MemChkOk
    .MemChkFail:
        mov dword [_dwMCRNumber],0
    .MemChkOk:
        
        ; 寻找 kernel.bin
        mov word [wSectorNo],SectorNoOfRootDirectory

        ; 软驱复位
        xor ah,ah
        xor dl,dl
        int 13h

    .search_in_root_dir_begin:
        cmp word [wRootDirSizeForLoop],0
        jz .no_kernel
        dec word [wRootDirSizeForLoop]
        mov ax,BaseOfKernelFile
        mov es,ax
        mov bx,OffsetOfKernelFile
        mov ax,[wSectorNo]
        mov cl,1
        call ReadSector
            
        mov si,KernelFileName               ; ds:si -> "KERNEL.BIN"
        mov di,OffsetOfKernelFile
        cld
        mov dx,10h
    .search_kernel:
        cmp dx,0
        jz .next_sector
        dec dx
        mov cx,11
    .cmp_filename:
        cmp cx,0
        jz .filename_found
        dec cx
        lodsb                               ; ds:si -> al
        cmp al,byte [es:di]                 ; if al == es:di
        jz .go_on
        jmp .different
    .go_on:
        inc di
        jmp .cmp_filename

    .different:
        and di,0FFE0h
        add di,20h        
        mov si,KernelFileName
        jmp .search_kernel

    .next_sector:
        add word [wSectorNo],1
        jmp .search_in_root_dir_begin
        
    .no_kernel:
        mov dh,2
        call DispStrRealMode
        jmp $                               ; 未找到 到此为止
    
    .filename_found:
        mov ax,RootDirSectors
        and di,0FFF0h                       ; di -> 当前条目的开始

        ; 保存 kernel.bin 的文件大小
        push eax
        mov eax,[es:di + 01Ch]
        mov dword [dwKernelSize],eax
        pop eax

        add di,01Ah                         ; di -> 首Sector
        mov cx,word [es:di]
        push cx                             ; 保存此 Sector 在 FAT 中的序号
        add cx,ax       
        add cx,DeltaSectorNo                ; cl <- loader.bin 起始扇区号
        mov ax,BaseOfKernelFile             ; es
        mov es,ax                           ; es <- BaseOfKernelFile
        mov bx,OffsetOfKernelFile           ; bx <- OffsetOfKernelFile
        mov ax,cx                           ; ax <- Sector 号

    .goon_loading_file:

        ; show Loading...
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
        pop ax                              ; 此 Sector 在FAT 中的序号
        call GetFATEntry
        cmp ax,0FFFh
        jz .file_loaded
        push ax                             ; 保存此 Sector 在 FAT 中的序号
        mov dx,RootDirSectors
        add ax,dx
        add ax,DeltaSectorNo
        add bx,[BPB_BytePerSec]
        jmp .goon_loading_file
    .file_loaded:
        call KillMotor                      ; 关闭驱动马达
    
        mov dh,1                            ; "Ready."
        call DispStrRealMode
       
 
        ; 准备进入保护模式
        
        ; 加载 GDTR
        lgdt    [GdtPtr]
        
        ; 关中断
        cli

        ; 打开地址线 A20
        in al,92h
        or al,00000010b
        out 92h,al

        ; 准备切换到保护模式
        mov eax,cr0
        or eax,1
        mov cr0,eax

        ; 真正进入保护模式
        jmp dword SelectorFlatC:(BaseOfLoaderPhyAddr+PM_START) ;



;=====================================================================
; 变量

wRootDirSizeForLoop     dw  RootDirSectors  ; root directory 占用的扇区数
wSectorNo               dw  0
b0dd                    db  0
dwKernelSize            dd  0

;=====================================================================
; 字符串
KernelFileName          db  "KERNEL  BIN",0 ; kernel.bin 的文件名
MessageLength           equ 9
LoadMessage:            db  "Loading  "
Message1                db  "Ready.   "
Message2                db  "No Kernel"
Message3                db  "TEST     "
;=====================================================================


;
; 显示字符串
; dh： 字符串序号
;
DispStrRealMode:
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
        add dh,3
        int 10h
        ret

;---------------------------------------------------------------------
;   从（Directory Entry 中的 Sector 号）为 ax 的 Sector 开始，
;   将 cl 个 Sector 读入 es：bx 中
;
ReadSector:
;                               |—— 柱面号 = y >> 1
;    扇区号          |——  商 y -|    
; -------------- = >-|          |—— 磁头号 = y & 1
;  每磁道扇区数      |
;                    |——  余 z => 起始扇区号 = z + 1
;
        push bp
        mov bp,sp
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
        ; 至此，柱面号，起始扇区，磁头号 全部得到
        mov dl,[BS_DrvNum]
    .go_on_reading:
        mov ah,2
        mov al,byte [bp-2]
        int 13h
        jc .go_on_reading

        add esp,2
        pop bp
        ret

;=====================================================================
; 找到序号为ax的 Sector 在 FAT 中的条目，结果放在 ax 中
;
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
        jz .even
        mov byte [b0dd],1
    .even:
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
        jnz .even2
        shr ax,4
    .even2:
        and ax,0FFFh

    .get_fat_enry_ok:
        pop bx
        pop es
        ret
        
;=====================================================================
; 关闭驱动马达
;
KillMotor:
        push dx 
        mov dx,03F2h
        mov al,0
        out dx,al
        pop dx
        ret

;---------------------------------------------------------------------
[SECTION .s32]
ALIGN   32
[bits 32]

PM_START:
        mov ax,SelectorVideo
        mov gs,ax
        
        mov ax,SelectorFlatRW
        mov ds,ax
        mov es,ax
        mov fs,ax
        mov ss,ax
        mov esp,TopOfStack

        push szMemChkTitle
        call DispStr
        add esp,4
        
        ; call DispMemInfo
        call SetupPaging

        ; mov ah,0Fh
        ; mov al,'P'
        ; mov [gs:((80 * 0 + 39) * 2)],ax

        ; jmp $
        call InitKernel
        jmp SelectorFlatC:KernelEntryPointPhyAddr

;---------------------------------------------------------------------
; 显示 AL 中的数字
DispAL:
        push ecx
        push edx
        push edi
    
        mov edi,[dwDispPos]

        mov ah,0Fh
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

; 显示一个整数型
DispInt:
        mov eax,[esp + 4]
        shr eax,24
        call DispAL

        mov eax,[esp + 4]
        shr eax,16
        call DispAL

        mov eax,[esp + 4]
        shr eax,8   
        call DispAL

        mov eax,[esp + 4]
        call DispAL

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
DispStr:
        push ebp
        mov ebp,esp
        push ebx
        push esi
        push edi

        mov esi,[ebp + 8]
        mov edi,[dwDispPos]
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
        mov [dwDispPos],edi

        pop edi
        pop esi
        pop ebx
        pop ebp
        ret
;---------------------------------------------------------------------
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


; 显示内存信息
DispMemInfo:
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
;---------------------------------------------------------------------
; 启动分页机制
SetupPaging:
        ; 根据内存大小计算应初始化多少PDE以及多少页表
        xor edx,edx
        mov eax,[dwMemSize]
        mov ebx,400000h             ; 400000h = 4M = 4096 * 1024
        div ebx
        mov ecx,eax                 ; 此时 ecx 为页表的个数，也即 PDE 应该的个数 
        test edx,edx
        jz .no_remainder
        inc ecx                     ; 如果余数不为0就增加一个页表
    .no_remainder:
        push ecx                    ; 页表个数
        
        ; 为简化处理，所有线性地址对应相等的物理地址，并且不考虑内存空间
        ; 初始化页目录
        mov ax,SelectorFlatRW
        mov es,ax
        mov edi,PageDirBase
        xor eax,eax
        mov eax,PageTblBase|PG_P|PG_USU|PG_RWW
    .1:
        stosd
        add eax,4096
        loop .1

        ; 在初始化所有页表
        pop eax
        mov ebx,1024
        mul ebx
        mov ecx,eax
        mov edi,PageTblBase
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
;---------------------------------------------------------------------
; 将 kernel.bin 的内容经过整理对齐后放到新的位置
; 遍历每一个 Program Header, 根据 Program Header 中的信息来确定把什么放到内存，放到什么位置，以及放多少
InitKernel:
        xor esi,esi
        mov cx,word [BaseOfKernelFilePhyAddr+2Ch]   ; ecx <- pELFHdr->e_phnum
        movzx ecx,cx    
        mov esi,[BaseOfKernelFilePhyAddr + 1Ch]     ; esi <- pELFHdr->e_phoff
        add esi,BaseOfKernelFilePhyAddr
    .begin:
        mov eax,[esi]
        cmp eax,0                           ; PT_NULL
        jz .no_action
        push dword [esi+010h]
        mov eax,[esi+04h]
        add eax,BaseOfKernelFilePhyAddr
        push eax
        push dword [esi+08h]
        call MemCpy
        add esp,12
    .no_action:
        add esi,020h
        dec ecx
        jnz .begin

        ret
        
;=====================================================================
        


[SECTION .data1]
ALIGN   32

DATA:
; 实模式下使用
; 字符串
_szMemChkTitle: db "BaseAddrL BaseAddrH LengthLow LengthHigh Type",0Ah,0
_szRAMSize:     db "RAM size",0
_szReturn:      db 0Ah,0
; 变量
_dwMCRNumber:   dd 0
_dwDispPos:     dd (80 * 6 + 0) * 2
_dwMemSize:         dd  0
; Address Range Descriptor structure
_ARDStruct:
    _dwBaseAddrLow: dd  0
    _dwBaseAddrHige:dd  0
    _dwLengthLow:   dd  0
    _dwLengthHige:  dd  0
    _dwType:        dd  0
_MemChkBuf:    times 256 db 0

; 保护模式下使用这些符号
szMemChkTitle           equ BaseOfLoaderPhyAddr + _szMemChkTitle   
szRAMSize               equ BaseOfLoaderPhyAddr + _szRAMSize      
szReturn                equ BaseOfLoaderPhyAddr + _szReturn       
dwDispPos               equ BaseOfLoaderPhyAddr + _dwDispPos      
dwMemSize               equ BaseOfLoaderPhyAddr + _dwMemSize      
dwMCRNumber             equ BaseOfLoaderPhyAddr + _dwMemSize      
ARDStruct               equ BaseOfLoaderPhyAddr + _ARDStruct      
    dwBaseAddrLow       equ BaseOfLoaderPhyAddr + _dwBaseAddrLow  
    dwBaseAddrHige      equ BaseOfLoaderPhyAddr + _dwBaseAddrHige 
    dwLengthLow         equ BaseOfLoaderPhyAddr + _dwLengthLow    
    dwLengthHige        equ BaseOfLoaderPhyAddr + _dwLengthHige   
    dwType              equ BaseOfLoaderPhyAddr + _dwType         

MemChkBuf               equ BaseOfLoaderPhyAddr + _MemChkBuf     

; 堆栈
StackSpace: times 1024 db  0
TopOfStack  equ BaseOfLoaderPhyAddr + $
