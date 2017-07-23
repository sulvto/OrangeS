        org     0100h
        
        BaseOfStack         equ     0100h
        
        ; kernel.bin 被加载的位置
        BaseOfKernelFile    equ     01000h  ; 段地址
        OffsetOfKernelFile  equ     0h      ; 偏移地址

        jmp start

; FAT12 磁盘的头
%include    "fat12hdr.inc"

start:
        mov ax,cs
        mov dx,ax
        mov es,ax
        mov ss,ax
        mov sp,BaseOfStack

        mov dh,0                           ; "Loading  "
        call DispStr

        ; 寻找 kernel.bin
        mov word [wSectorNo],SectorNoOfRootDirectory

        ; 软驱复位
        xor ah,ah
        xor dl,dl
        int 13h

    .search_in_root_dir_begin:
        cmp word [wRootDirSizeForLoop],0
        jz .no_kernel
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
        call DispStr
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
        call DispStr
        jmp $

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
Message1:               db  "Ready.   "
Message2:               db  "No Kernel"
;=====================================================================


;
; 显示字符串
; dh： 字符串序号
;
DispStr:
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
