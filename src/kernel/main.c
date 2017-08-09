//
// Created by sulvto on 17-7-30.
//

#include    "type.h"
#include    "const.h"
#include    "protect.h"
#include    "proto.h"
#include    "string.h"
#include    "proc.h"
#include    "global.h"




PUBLIC int kernel_main() {
    disp_str("------------------\"kernel_main\" begins--------------------\n");
   
    TASK*       p_task          = task_table;     
    PROCESS*    p_proc          = proc_table;
    char*       p_task_stack    = task_stack + STACK_SIZE_TOTAL;
    u16         selector_ldt    = SELECTOR_LDT_FIRST;
    for(int i=0; i<NR_TASKS; i++) {
        strcpy(p_proc->p_name,p_task->name);
        p_proc->pid = i;
        p_proc->ldt_sel = selector_ldt;
        p_proc->ldt_sel = SELECTOR_LDT_FIRST;

        memcpy(&p_proc->ldts[0],&gdt[SELECTOR_KERNEL_CS >> 3],sizeof(DESCRIPTOR));
        p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;  // change the DPL
        memcpy(&p_proc->ldts[1],&gdt[SELECTOR_KERNEL_DS >> 3],sizeof(DESCRIPTOR));
        p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;  // change the DPL
        
        //  SA_RPL_MASK   0xFFFC      11111111 11111100
        //  SA_TI_MASK    0xFFFB      11111111 11111011
        //  SA_TIL        4           00000000 00000100
        //  RPL_TASK      SA_RPL1     00000000 00000001
        //  SELECTOR_KERNEL_GS                    11011
        p_proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | RPL_TASK;            // 11001 
        p_proc->regs.fs = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;       // 1101   
        p_proc->regs.es = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;       // 1101 
        p_proc->regs.ds = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
        p_proc->regs.eip = (u32)p_task->init_eip;                                              //
        p_proc->regs.cs = (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;        // 111 
        p_proc->regs.eflags = 0x1202;                                                // IF = 1,IOPL = 1, bit 2 is always 1.
        p_proc->regs.esp = (u32)p_task_stack;
        p_proc->regs.ss = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;

        p_task_stack -= p_task->stacksize;
        p_proc++;
        p_task++;
        selector_ldt += 1<<3;
    }
    k_reenter = 0;    
    ticks = 0;

    p_proc_ready  = proc_table;

    // 初始化 8253 PIT
    out_byte(TIMER_MODE,RATE_GENERATOR);
    out_byte(TIMER0,(u8)(TIMER_FREQ/HZ));
    out_byte(TIMER0,(u8)((TIMER_FREQ/HZ) >> 8));


    put_irq_handler(CLOCK_IRQ,clock_handler);
    enable_irq(CLOCK_IRQ);

    restart();

    while(1){}
}

void TestA() {
    while(1) {
        
        disp_str("A");
        disp_int(get_ticks());
        disp_str(".");
        milli_delay(1000);
    }
}

void TestB() {
    int i=0X1000;
    while(1) {
        disp_str("B");
        disp_int(i++);
        disp_str(".");
        milli_delay(1000);
    }
}

void TestC() {
    int i=0X2000;
    while(1) {
        disp_str("C");
        disp_int(i++);
        disp_str(".");
        milli_delay(1000);
    }
}
