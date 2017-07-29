//
// Created by sulvto on 17-7-28.
//

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"

PUBLIC void init_8259A() {
    // Master 8259, ICW1.
    out_byte(INT_M_CTL, 0x11);

    // Slave 8259, ICW1.
    out_byte(INT_S_CTL, 0x11);

    // Master 8259, ICW2.
    out_byte(INT_M_CTLMASK, INT_VECTOR_IRQ0);

    // Slave 8259, ICW2.
    out_byte(INT_S_CTLMASK, INT_VECTOR_IRQ8);

    // Master 8259, ICW3.
    out_byte(INT_M_CTLMASK, 0x4);

    // Slave 8259, ICW3.
    out_byte(INT_S_CTLMASK, 0x2);

    // Master 8259, ICW4.
    out_byte(INT_M_CTLMASK, 0x1);

    // Slave 8259, ICW4.
    out_byte(INT_S_CTLMASK, 0x1);

    // Master 8259, OCW1.
    out_byte(INT_M_CTLMASK, 0xFD);

    // Slave 8259, OCW1.
    out_byte(INT_S_CTLMASK, 0xFF);
};

PUBLIC void spurious_irq(int irq) {
    disp_str("spurious_irq: ");
    disp_int(irq);
    disp_str("\n");
}
