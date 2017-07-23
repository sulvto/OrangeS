%macro  Descriptor 3
        dw      %2 & 0FFFFh                         ; 段界限1
        dw      %1 & 0FFFFh                         ; 段基址1   
        db      (%1 >> 16) & 0FFh                   ; 段基址2
        dw      ((%2 >> 8) & 0F00h) | (%3 & 0F0FFh) ; 属性1 + 段界限2 + 属性2
        db      (%1 >> 24) & 0FFh                   ; 段基址3
%endmacro

; 描述符类型
DA_32       equ     4000h       ; 32位段

DA_DPL0     equ     00h         ; DPL = 0
DA_DPL1     equ     20h         ; DPL = 1
DA_DPL2     equ     40h         ; DPL = 2
DA_DPL3     equ     60h         ; DPL = 3

; 存储段描述符类型
DA_DR       equ     90h         ; 存在的
