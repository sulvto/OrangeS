//
// Created by sulvto on 17-7-28.
//

#ifndef _ORANGES_CONST_H_
#define _ORANGES_CONST_H_

#define PUBLIC
#define PRAVITE static

/*
 * GDT 和 IDT 中描述符的个数
 */
#define GDT_SIZE    128

// 8259A
#define  INT_M_CTL  0x20
#define  INT_M_CTLMASK 0x21
#define  INT_S_CTL 0xA0
#define  INT_S_CTLMASK 0xA1

#endif
