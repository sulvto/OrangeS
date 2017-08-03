
; FAT12 磁盘的头
BS_OEMName      DB 'ForrestY'
BPB_BytsPerSec  DW 512          ; 扇区字节数
BPB_SecPerClus  DB 1            ; 族扇区数
BPB_RevdSecCnt  DW 1            ; Boot 记录占用扇区数
BPB_NumFATs     DB 2            ; 2 FAT 表
BPB_RootEntCnt  DW 224          ; 根目录文件数最大值
BPB_TotSec16    DW 2880         ; 逻辑扇区总数
BPB_Media       DB 0xF0         ; 媒体描述符
BPB_FATSz16     DW 9            ; FAT扇区数
BPB_SecPerTrk   DW 18           ; 磁道扇区数
BPB_NumHeads    DW 2            ; 磁头数
BPB_HiddSec     DD 0            ; 隐藏扇区数
BPB_TotSec32    DD 0            ; 
BS_DrvNum       DB 0            ; 中断 13 的驱动器号
BS_Reservedl    DB 0            ; 未使用
BS_BootSig      DB 29h          ; 扩展引导标记 29h
BS_VolID        DD 0            ; 卷序列号
BS_VolLab       DB 'MyOs0.01  ' ; 卷标，必须是11字节    
BS_FileSysType  DB 'FAT12   '   ;


; 基于 FAT12 头的一些常量定义
; BPB_FATSz16
FATSz                   equ 9
RootDirSectors          equ 14      ; 根目录占用空间
SectorNoOfRootDirectory equ 19      ; Root Directory 的第一个扇区号
SectorNoOfFAT1          equ 1       ; FAT1 的第一个扇区号 = BPB_RsvdSecCnt
DeltaSectorNo           equ 17      ; DeltaSectorNo = BPB_RsvdSecCnt + （BPB_NumFATs * FATSz） - 2
