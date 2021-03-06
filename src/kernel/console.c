//
// Created by sulvto on 17-8-13.
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
#include "keyboard.h"
#include "proto.h"

PRIVATE void set_cursor(unsigned int position);
PRIVATE void set_video_start_addr(u32 addr);
PRIVATE void flush(CONSOLE* p_console);


PUBLIC void init_screen(TTY* p_tty) {
    int nr_tty = p_tty - tty_table;
    p_tty->p_console = console_table + nr_tty;

    int v_mem_size = V_MEM_SIZE >> 1;       // 显存总大小 （in word）

    int con_v_mem_size = v_mem_size / NR_CONSOLES;
    p_tty->p_console->original_addr       = nr_tty * con_v_mem_size;
    p_tty->p_console->v_mem_limit         = con_v_mem_size;
    p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;

    // 默认光标位置在最开始处
    p_tty->p_console->cursor = p_tty->p_console->original_addr;

    if (nr_tty == 0) {
        // 第一个控制台沿用原来的光标位置
        p_tty->p_console->cursor = disp_pos / 2;
        disp_pos = 0;
    }else {
        out_char(p_tty->p_console,nr_tty + '0');
        out_char(p_tty->p_console,'#');
    }
    set_cursor(p_tty->p_console->cursor);
}

PUBLIC int is_current_console(CONSOLE* p_console) {
    return (p_console == &console_table[nr_current_console]);
}

PUBLIC void out_char(CONSOLE* p_console,char ch) {
    u8* p_vmem = (u8*)(V_MEM_BASE + p_console->cursor * 2);
    switch (ch) {
        case '\n':
            if (p_console->cursor < p_console->original_addr 
                                    + p_console->v_mem_limit 
                                    - SCR_WIDTH) {
                p_console->cursor = p_console->original_addr + SCR_WIDTH * 
                            ((p_console->cursor - p_console->original_addr)
                            / SCR_WIDTH + 1);
            }
            break;
        case '\b':
            if (p_console->cursor > p_console->original_addr) {
                p_console->cursor--;
                *(p_vmem - 2) = ' ';
                *(p_vmem - 1) = DEFAULT_CHAR_COLOR;
            }
            break;
        default:
            if (p_console->cursor < 
                p_console->original_addr + p_console->v_mem_limit -1) {
                *p_vmem++ = ch;                               
                *p_vmem++ = DEFAULT_CHAR_COLOR;
                p_console->cursor++;
            }
            break;
    }

    while (p_console->cursor >= p_console->current_start_addr + SCR_SIZE) {
        scroll_screen(p_console,SCR_DN);
    }
    
    flush(p_console);
}

PRIVATE void flush(CONSOLE* p_console) {
    if (is_current_console(p_console)) {
        set_cursor(p_console->cursor);
        set_video_start_addr(p_console->current_start_addr);    
    }
    
    
}

PRIVATE void set_cursor(unsigned int position) {
    disable_int();
    out_byte(CRTC_ADDR_REG, CURSOR_H);
    out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
    out_byte(CRTC_ADDR_REG, CURSOR_L);
    out_byte(CRTC_DATA_REG, position & 0xFF);
    enable_int();
}

PRIVATE void set_video_start_addr(u32 addr) {
    disable_int();
    out_byte(CRTC_ADDR_REG, START_ADDR_H);
    out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
    out_byte(CRTC_ADDR_REG, START_ADDR_L);
    out_byte(CRTC_DATA_REG, addr & 0xFF);
    enable_int();
}

PUBLIC void select_console(int nr_console) {
    if ((nr_console < 0) || (nr_console >= NR_CONSOLES)) {
        return;
    }

    nr_current_console = nr_console;

    flush(&console_table[nr_console]);
}

/**
 * SCR_UP: 向上滚屏
 * SCR_DN: 向下滚屏
 *
 *
 */
PUBLIC void scroll_screen(CONSOLE* p_console, int direction) {
    if (direction == SCR_UP) {
        if(p_console->current_start_addr > p_console->original_addr) {
            p_console->current_start_addr -= SCR_WIDTH;
        } 
    }else if (direction == SCR_DN) {
        if(p_console->current_start_addr + SCR_SIZE < p_console->original_addr + p_console-> v_mem_limit) {       
            p_console->current_start_addr += SCR_WIDTH;
        } 
    }else {
    }

    flush(p_console);
}

