//
// Created by sulvto on 17-7-28.
//

#ifndef _ORANGES_PROTECT_H_
#define _ORANGES_PROTECT_H_

typedef struct s_descriptor {
    u16     limit_low;          // Limit
    u16     base_low;           // Base
    u8      base_mid;           // Base
    u8      attr1;              // P（1） DPL（2） DT（1） TYPE（4）
    u8      limit_high_attr2;   // G（1） D（1） 0（1） AVL（1） LimitHigh(4)
    u8 base_high;          // Base
} DESCRIPTOR;

// 门描述符
typedef struct s_gate {
    u16     offset_low;           // Offset low
    u16     selector;             // Selector
    u8      dcount;               // 
    u8      attr;                 // P（1） DPL（2） DT（1） TYPE（4）
    u16     offset_high;          // Offset high
} GATE;


typedef struct s_tss {
    u32     backlink;
    u32     esp0;
    u32     ss0;
    u32     esp1;
    u32     ss1;
    u32     esp2;
    u32     ss2;
    u32     cr3;
    u32     eip;
    u32     flags;
    u32     eax;
    u32     ecx;
    u32     edx;
    u32     ebx;
    u32     esp;
    u32     ebp;
    u32     esi;
    u32     edi;
    u32     es;
    u32     cs;
    u32     ss;
    u32     ds;
    u32     fs;
    u32     gs; 
    u32     ldt;
    u16     trap;
    u16     iobase;
}TSS;


// GDT
// 描述符索引
#define INDEX_DUMMY      0
#define INDEX_FLAT_C     1
#define INDEX_FLAT_RW    2
#define INDEX_VIDEO      3
#define INDEX_TSS        4
#define INDEX_LDT_FIRST  5

#define SELECTOR_DUMMY         0
#define SELECTOR_FLAT_C     0x08
#define SELECTOR_FLAT_RW    0x10
#define SELECTOR_VIDEO      (0x18+3)
#define SELECTOR_TSS        0x20
#define SELECTOR_LDT_FIRST  0x28

#define SELECTOR_KERNEL_CS  SELECTOR_FLAT_C
#define SELECTOR_KERNEL_DS  SELECTOR_FLAT_RW
#define SELECTOR_KERNEL_GS  SELECTOR_VIDEO

// 每个 LDT 中的描述符个数
#define LDT_SIZE        2

// 选择子类型值
#define SA_RPL_MASK     0xFFFC
#define SA_RPL0         0
#define SA_RPL1         1
#define SA_RPL2         2
#define SA_RPL3         3

#define SA_TI_MASK      0xFFFB
#define SA_TIG          0
#define SA_TIL          4

// 描述符类型值
#define DA_32               0x4000 // 32 位段
#define DA_LIMIT_4K         0x8000 // 段界限粒度为 4K 字节
#define DA_DPL0             0x00   // DPL = 0
#define DA_DPL1             0x20   // DPL = 1
#define DA_DPL2             0x40   // DPL = 2
#define DA_DPL3             0x60   // DPL = 3
// 存储段描述符类型值
#define DA_DR               0x90   
#define DA_DRW              0x92
#define DA_DRWA             0x93   
#define DA_C                0x98
#define DA_CR               0x9A
#define DA_CC0              0x9C
#define DA_CC0R             0x9E
// 系统段描述符类型值
#define DA_LDT              0x82
#define DA_TaskGate         0x85
#define DA_386TSS           0x89
#define DA_386CGate         0x8C
#define DA_386IGate         0x8E
#define DA_386TGate         0x8F

// 中断向量
#define INT_VECTOR_DIVIDE       0x0
#define INT_VECTOR_DEBUG        0x1
#define INT_VECTOR_NMI          0x2
#define INT_VECTOR_BREAKPOINT   0x3
#define INT_VECTOR_OVERFLOW     0x4
#define INT_VECTOR_BOUNDS       0x5
#define INT_VECTOR_INVAL_OP     0x6
#define INT_VECTOR_COPROC_NOT   0x7
#define INT_VECTOR_DOUBLE_FAULT 0x8
#define INT_VECTOR_COPROC_SEG   0x9
#define INT_VECTOR_INVAL_TSS    0xA
#define INT_VECTOR_SEG_NOT      0xB
#define INT_VECTOR_STACK_FAULT  0xC
#define INT_VECTOR_PROTECTION   0xD
#define INT_VECTOR_PAGE_FAULT   0xE
#define INT_VECTOR_COPROC_ERR   0x10

     
// 中断向量
#define INT_VECTOR_IRQ0     0x20
#define INT_VECTOR_IRQ8     0x28

// 宏
// 线性地址 -> 物理地址
#define vir2phys(seg_base, vir) (u32)(((u32)seg_base) + (u32)(vir))

#endif
