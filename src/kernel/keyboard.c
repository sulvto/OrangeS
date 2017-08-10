//
// Created by sulvto on 17-8-10.
//

PRIVATE KB_INPUT    kb_in;

PUBLIC void init_keyboard() {
    kb_in.count = 0;
    kb_in.p_head= kb_in.p_tail = kb_in.buf;

    put_irq_handler(KEYBOARD_IRQ, keyboard_handler);// 设置键盘中断处理程序
    enable_irq(KEYBOARD_IRQ);                       // 开键盘中断
}


PUBLIC void keyboard_handler(int irq) {
    u8 scan_code = in_byte(KB_DATA);
    
    if (kb_in.count < KB_IN_BYTES) {
        *(kb_in.p_head) = scan_code;
        kb_in.p_head++;
        if(kb_in.p_head == kb_in.buf + KB_IN_BYTES) {
            kb_in.p_head = kb_in.buf;
        }
        kb_in.count++;
    }
}
