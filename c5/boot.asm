org 07c00h
 
BaseOfStack     equ 07c00h 

%include "load.inc"

jmp short LABEL_START
nop                    ;; ???

%include    "fat12hdr.inc"

LABEL_START:
        mov ax,cs
        mov ds,ax
        mov es,ax
        mov ss,ax
        mov sp,BaseOfStack

        ; 清屏
        mov ax,0600h        ; AH = 6, AL = 0h
        mov bx,0700h        ; 黑底白字（BL = 07h）
        mov cx,0            ; 左上角：（0，0）
        mov dx,0184fh       ; 右下角：（80，50）
        int 10h
        
        mov dh,0            ; "Booting  "
        call DispStr        ; 显示

        xor ah,ah   ; 清零
        xor dl,dl   ; 清零
        int 13h

        ; 寻找loader.bin
        mov word [wSectorNo],SectorNoOfRootDirectory
        

LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
        cmp word [wRootDirSizeForLoop],0
        jz LABEL_NO_LOADERBIN
        dec word [wRootDirSizeForLoop]
        mov ax,BaseOfLoader
        mov es,ax               ; es <-- BaseOfLoader
        mov bx,OffsetOfLoader   ; bx <-- OffsetOfLoader
        mov ax,[wSectorNo]
        mov cl,1
        call ReadSector

        mov si,LoaderFileName
        mov di,OffsetOfLoader
        cld
        mov dx,10h

LABEL_SEARCH_FOR_LOADERBIN:
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
        mov si,LoaderFileName
        jmp LABEL_SEARCH_FOR_LOADERBIN

LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR:
        add word [wSectorNo],1
        jmp LABEL_SEARCH_IN_ROOT_DIR_BEGIN

LABEL_NO_LOADERBIN:
        mov dh,2
        call DispStr
        jmp $       ;没有找到，死循环在这里

LABEL_FILENAME_FOUND:
        mov ax,RootDirSectors
        and di,0FFE0h
        add di,01Ah
        mov cx,word [es:di]
        push cx
        add cx,ax
        add cx,DeltaSectorNo
        mov ax,BaseOfLoader
        mov es,ax
        mov bx,OffsetOfLoader
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
        mov dh,1
        call DispStr
        
        jmp BaseOfLoader:OffsetOfLoader


wRootDirSizeForLoop dw RootDirSectors
wSectorNo           dw 0                ; 要读的扇区号
b0dd                db 0                ; 

LoaderFileName      db  "LOADER  BIN",0
MessageLength       equ 9
BootMessage:        db  "[Booting]"
Message1            db  "Ready.   "
Message2            db  "No Loader"

DispStr:      ; 显示字符串
        mov ax,MessageLength
        mul dh
        add ax,BootMessage
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
        mov ax,BaseOfLoader
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
        
LABEL_GET_FAT_ENRY_OK:
        pop bx
        pop es
        ret

times 510-($-$$)    db 0    ; 填充剩下的空间
dw  0xaa55
