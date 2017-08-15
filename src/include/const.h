//
// Created by sulvto on 17-7-28.
//

#ifndef _ORANGES_CONST_H_
#define _ORANGES_CONST_H_

#define PUBLIC
#define PRIVATE static

#define EXTERN extern

/*
 * GDT 和 IDT 中描述符的个数
 */
#define GDT_SIZE    128
#define IDT_SIZE    256

// 权限
#define PRIVILEGE_KRNL  0
#define PRIVILEGE_TASK  1
#define PRIVILEGE_USER  3

// RPL
#define RPL_KRNL    SA_RPL0
#define RPL_TASK    SA_RPL1
#define RPL_USER    SA_RPL3

// 8259A
#define  INT_M_CTL      0x20
#define  INT_M_CTLMASK  0x21
#define  INT_S_CTL      0xA0
#define  INT_S_CTLMASK  0xA1

// 8253/8254 PIT
#define  TIMER0         0x40
#define  TIMER_MODE     0x43
#define  RATE_GENERATOR 0x34

#define  TIMER_FREQ     1193182L
#define  HZ             100

// TTY
#define NR_CONSOLES     3

// AT keyboard
// 8042 ports
#define  KB_DATA    0x60
#define  KB_CMD     0x64

#define  NR_IRQ     16
#define  CLOCK_IRQ  0
#define  KEYBOARD_IRQ   1

// VGA
#define CRTC_ADDR_REG   0x3D4       // CRT Controller Registers - Addr Registers
#define CRTC_DATA_REG   0x3D5       // CRT Controller Registers - Data Registers
#define START_ADDR_H    0xC         // reg index of video mem start addr (MSB)
#define START_ADDR_L    0xD         // reg index of video mem start addr (LSB)
#define CURSOR_H        0xE         // reg index of cursor position (MSB)
#define CURSOR_L        0xF         // reg index of cursor position (LSB)
#define V_MEM_BASE      0xB8000     // base of color video memory
#define V_MEM_SIZE      0x8000      // 32K: B8000H -> BFFFFH

#define LED_CODE        0xED
#define KB_ACK          0xFA

#define NR_SYS_CALL     2

#endif

