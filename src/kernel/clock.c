//
// Created by sulvto on 17-8-5.
//

#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

PUBLIC void init_clock() {
    // 初始化 8253 PIT     
    out_byte(TIMER_MODE, RATE_GENERATOR);
    out_byte(TIMER0, (u8)(TIMER_FREQ / HZ));
    out_byte(TIMER0, (u8)((TIMER_FREQ / HZ) >> 8));

    put_irq_handler(CLOCK_IRQ, clock_handler);   // 设置始终中断
    enable_irq(CLOCK_IRQ);                      // 让8259A可以接收时钟中断
}

PUBLIC void clock_handler(int irq) {

    ticks++;

    p_proc_ready->ticks--;

    if (k_reenter != 0) {
        return;
    }

    if (p_proc_ready->ticks > 0) {
        return;
    }

    if (key_pressed) inform_int(TASK_TTY);

    schedule();

}

PUBLIC void milli_delay(int milli_sec) {
    int t = get_ticks();
    while (((get_ticks() - t) * 1000 / HZ) < milli_sec) { }
}
