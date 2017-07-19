; 描述符
; usage:Descriptor Base,Limit,Attr
;       Base:   dd
;       Limit:  dd
;       Attr:   dw
;


; 描述符类型
DA_32       equ     4000h       ; 32 位段

DA_DPL0     equ     00h         ; DPL = 0
DA_DPL1     equ     20h         ; DPL = 1
DA_DPL2     equ     40h         ; DPL = 2
DA_DPL3     equ     60h         ; DPL = 3

; 存储段描述符类型
DA_DR       equ     90h         ; 存在的只读数据段类型值
DA_DRW      equ     92h         ; 存在的可读写数据段属性值
DA_DRWA     equ     93h         ; 存在的已访问可读写数据段类型值
DA_C        equ     98h         ; 存在的只执行代码段属性值
DA_CR       equ     9Ah         ; 存在的可执行可读代码段属性值
DA_CCO      equ     9Ch         ; 存在的只执行一致代码段属性值
DA_CCOR     equ     9Eh         ; 存在的可执行可读一致代码段属性值

; 系统段描述符类型
DA_LDT      equ     82h         ; 局部描述符表类型值
DA_TaskGate equ     85h         ; 任务门类型值
DA_386T88   equ     89h         ; 可用 386 任务状态段类型值
DA_386CGate equ     8Ch         ; 386 调用门类型值
DA_386IGate equ     8Eh         ; 386 中断门类型值
DA_386TGate equ     8Fh         ; 386 陷阱门类型值


; SL_ : Selector Attribute
SA_RPL0     equ 0
SA_RPL1     equ 1
SA_RPL2     equ 2
SA_RPL3     equ 3

SA_TIG      equ 0
SA_TIL      equ 4


%macro  Descriptor 3
         dw     %2 & 0FFFFh                         ; 段界限1
         dw     %1 & 0FFFFh                         ; 段基地址1
         db     (%1 >> 16) & 0FFh                   ; 段基地址2
         dw     ((%2 >> 8) & 0F00h) | (%3 & 0F0FFh) ; 属性1 + 段界限2+属性2
         db     (%1 >> 24) & 0FFh                   ; 段基地址3
%endmacro
