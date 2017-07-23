; 描述符类型
DA_32       equ     4000h       ; 32位段
DA_LIMIT_4K equ     8000h

DA_DPL0     equ     00h         ; DPL = 0
DA_DPL1     equ     20h         ; DPL = 1
DA_DPL2     equ     40h         ; DPL = 2
DA_DPL3     equ     60h         ; DPL = 3

; 存储段描述符类型
DA_DR       equ     90h         ; 存在的
DA_DRW      equ     92h         ;
DA_DRWA     equ     93h         ; 
DA_C        equ     98h         ; 
DA_CR       equ     9Ah         ; 
DA_CCO      equ     9Ch         ; 
DA_CCOR     equ     9Eh         ; 

; 系统段描述符
DA_LDT      equ     82h
DA_TaskGate equ     85h
DA_386TSS   equ     89h
DA_386CGate equ     8Ch
DA_386IGate equ     8Eh
DA_386TGate equ     8Fh

; 选择子类型值说明
SA_RPL0     equ     0
SA_RPL1     equ     1
SA_RPL2     equ     2
SA_RPL3     equ     3

SA_TIG      equ     0
SA_TIL      equ     4


; 分页机制使用的常量说明
PG_P        EQU     1
PG_RWR      EQU     0
PG_RWW      EQU     2
PG_USS      EQU     0
PG_USU      EQU     4


; 描述符
%macro  Descriptor 3
        dw      %2 & 0FFFFh                         ; 段界限1
        dw      %1 & 0FFFFh                         ; 段基址1   
        db      (%1 >> 16) & 0FFh                   ; 段基址2
        dw      ((%2 >> 8) & 0F00h) | (%3 & 0F0FFh) ; 属性1 + 段界限2 + 属性2
        db      (%1 >> 24) & 0FFh                   ; 段基址3
%endmacro   ; 共 8 字节

; 门
%macro Gate 4
        dw (%2 & 0FFFFh)
        dw  %1
        dw (%3 & 1Fh) | ((%4 << 8) & 0FF00h)
        dw ((%2 >> 16) & 0FFFFh)
%endmacro ; 共 8 字节
