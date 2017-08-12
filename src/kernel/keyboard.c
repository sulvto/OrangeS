//
// Created by sulvto on 17-8-10.
//

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "keyboard.h"
#include "keymap.h"

PRIVATE KB_INPUT    kb_in;

PUBLIC void keyboard_handler(int irq) {
    u8 scan_code = in_byte(KB_DATA);
    if (kb_in.count < KB_IN_BYTES) {
        *(kb_in.p_head) = scan_code;
        kb_in.p_head++;
        if (kb_in.p_head == kb_in.buf + KB_IN_BYTES) {
            kb_in.p_head = kb_in.buf;
        }
        kb_in.count++;
    }
}

PUBLIC void init_keyboard() {
    kb_in.count = 0;
    kb_in.p_head= kb_in.p_tail = kb_in.buf;

    put_irq_handler(KEYBOARD_IRQ, keyboard_handler);// 设置键盘中断处理程序
    enable_irq(KEYBOARD_IRQ);                       // 开键盘中断
}

PUBLIC void keyboard_read() {
    u8      scan_code;
    char    output[2];
    int     make;           // TRUE:make; FALSE:break;
    if (kb_in.count > 0) {
        disable_int();
        scan_code = *(kb_in.p_tail);
        kb_in.p_tail++;
        if (kb_in.p_tail == kb_in.buf + KB_IN_BYTES) {
            kb_in.p_tail = kb_in.buf;
        }
        kb_in.count--;
        enable_int();

        if (scan_code == 0xE1) {
            // TODO
        }else if (scan_code == 0xE0) {
            // TODO
        }else {
            // Make Code or Break Code
            make = (scan_code & FLAG_BREAK ? FALSE : TRUE);
            if (make) {
                output[0] = keymap[(scan_code & 0x7F)*MAP_COLS]; // 0x7F -> 1111111
                disp_str(output);
            }
        }


    //    disp_int(scan_code);
    }
}
