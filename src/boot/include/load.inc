KernelEntryPointPhyAddr equ 0x1000

PageDirBase     equ 0x100000 ; 页目录开始地址: 2M
PageTblBase     equ 0x101000 ; 页表开始地址:   2M + 4K

BaseOfLoader        equ 0x9000              ; LOADER.BIN 被加载到的位置  ---- 段地址
OffsetOfLoader      equ 0x100               ; LOADER.BIN 被加载到的位置  ---- 偏移地址
BaseOfLoaderPhyAddr equ BaseOfLoader * 0x10  ; LOADER.BIN 被加载到的位置  ---- 物理地址
       

KERNEL_FILE_SEG equ 0x7000                        ;  KERNEL.BIN 被加载到的位置  ---- 段地址
KERNEL_FILE_OFF equ 0                              ;  KERNEL.BIN 被加载到的位置  ---- 偏移地址
KERNEL_FILE_PHY_ADDR equ KERNEL_FILE_SEG * 0x10

KERNEL_VALID_SPACE equ  BaseOfLoaderPhyAddr - KERNEL_FILE_PHY_ADDR

; 
BOOT_PARAM_ADDR	equ 0x900
BOOT_PARAM_MAGIC	equ 0xB007
