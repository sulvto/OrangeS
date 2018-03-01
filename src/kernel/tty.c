//
// Created by sulvto on 17-8-12.
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

#define TTY_FIRST   (tty_table)
#define TTY_END     (tty_table + NR_CONSOLES)
PRIVATE void init_tty(TTY* p_tty);
PRIVATE void tty_do_read(TTY* tty, MESSAGE* msg);
PRIVATE void tty_do_write(TTY* tty, MESSAGE* msg);
PRIVATE void put_key(TTY* p_tty,u32 key);

PUBLIC void task_tty() {
    TTY* tty;
    MESSAGE msg;

    init_keyboard();

    for (tty=TTY_FIRST; tty<TTY_END; tty++) {
        init_tty(tty);
    }

    select_console(0);
    while (1) {
        for (tty=TTY_FIRST; tty<TTY_END; tty++) {
            do {
                tty_dev_read(tty);
                tty_dev_write(tty);
            } while (tty.inbuf_count);
        }

        send_recv(RECEVIE, ANY, &msg);
        int src = msg.source;
        assert(src != TASK_TTY);
        TTY* ptty = &tty_table[msg.DEVICE];

        switch (msg.type) {
            case DEV_OPEN:
                reset_msg(&msg);
                msg.type = SYSCALL_RET;
                send_recv(SEND, src, &msg);
                break;
            case DEV_READ:
                tty_do_read(ptty, &msg);
                break;
            case DEV_WRITE:
                tty_do_write(ptty, &msg);
                break;
            case HEAD_INT:
                /**
                 *  waked up by clock_handler -- a key was just pressed
                 *  @see clock_handler() inform_int()
                 *
                 */
                key_pressed = 0;
                continue;
            default:
                dump_msg("TTY::unknown msg", &msg);
                break;
        }
    }
}


PUBLIC void tty_write(TTY* p_tty, char* buf, int len) {
    char* p = buf;
    int i = len;
    while (i) {
        out_char(p_tty->p_console,*p++);
        i--;
    }
}

PUBLIC int sys_write(char* buf, int len, struct proc* p_proc) {
    tty_write(&tty_table[p_proc->nr_tty], buf, len);
    return 0;
}

PRIVATE void init_tty(TTY* p_tty) {
    p_tty->inbuf_count = 0;
    p_tty->inbuf_head = p_tty->inbuf_tail = p_tty->in_buf;

    init_screen(p_tty);
}

PUBLIC void in_process(TTY* p_tty, u32 key) {
    char output[2] = {'\0', '\0'};

    if (!(key & FLAG_EXT)) {
        put_key(p_tty, key);
    }else {
        int raw_code = key & MASK_RAW;
        switch (raw_code) {
            case ENTER:
                put_key(p_tty, '\n');
                break;
            case BACKSPACE:
                put_key(p_tty, '\b');
                break;
            case UP:
                if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
                    scroll_screen(p_tty->p_console,SCR_DN);
                }
                break;
            case DOWN:
                if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
                    scroll_screen(p_tty->p_console,SCR_UP);
                }
                break;
            case F1:
            case F2:
            case F3:
            case F4:
            case F5:
            case F6:
            case F7:
            case F8:
            case F9:
            case F10:
            case F11:
            case F12:
                    // Alt + F1-F12
                    if ((key & FLAG_ALT_L) || (key & FLAG_ALT_R)) {
                        select_console(raw_code - F1);
                    }
                    break;
            default:
                break;
        }
    }
}

PRIVATE void put_key(TTY* p_tty, u32 key) {
    if (p_tty->inbuf_count < TTY_IN_BYTES) {
       *(p_tty->inbuf_head) = key;
        p_tty->inbuf_head++;
        if (p_tty->inbuf_head == p_tty->in_buf + TTY_IN_BYTES) {
            p_tty->inbuf_head = p_tty->in_buf;
        }
        p_tty->inbuf_count++;
    }
}

/**
 * Get chars from the keyboard buffer if the TTY::console is the 'current' console.
 *
 * @see keyboard_read()
 *
 * @param tty Ptr to TTY
 */
PEIVATE void tty_dev_read(TTY* tty) {
    if (is_current_console(tty->p_console)) keyboard_read(tty);
}

/**
 * Echo the char just pressed and transfer it to the waiting process.
 *
 * @param tty   Ptr to a TTY struct.
 *
 */
PRIVATE void tty_dev_write(TTY* tty) {
    while (p_tty->inbuf_count) {
        char ch = *(p_tty->inbuf_tail);
        p_tty->inbuf_tail++;
        if (p_tty->inbuf_tail == p_tty->in_buf + TTY_IN_BYTES) {
            p_tty->inbuf_tail = p_tty->in_buf;
        }
        p_tty->inbuf_count--;

        if (tty->tty_left_cnt) {
            if (ch >= ' ' && ch <= '~') {   // printable (see ascii)
                out_char(p_tty->p_console,ch);
                void* p = tty->tty_req_buf + tty->tty_trans_cnt;
                phys_copy(p, (void *)va2la(TASK_TTY, &ch), 1);
                tty->tty_trans_cnt++;
                tty->tty_left_cnt--;
            } else if (ch == '\b' && tty->tty_trans_cnt) {
                out_char(p_tty->p_console,ch);
                tty->tty_trans_cnt--;
                tty->tty_left_cnt++;
            } else if (ch == '\n' || tty->tty_left_cnt == 0) {
                out_char(p_tty->p_console,ch);
                MESSAGE msg;
                msg.type = RESUME_PROC;
                msg.PROC_NR = tty->tty_proc_nr;
                msg.CNT = tty->tty_trans_cnt;
                send_recv(SEND, tty->tty_caller, &msg);
                tty->tty_left_cnt = 0;
            }
        }
    }
}

/**
 * Invoked when task TTY receives DEV_READ message.
 *
 * @note The routine will return immediately after setting some members of
 * TTY struct, telling FS to suspend the proc who wants to read. The real
 * transfer (tty buffer -> proc buffer) is not done here.
 *
 * @param tty From which TTY the caller proc wants to read.
 * @param msg The MESSAGE just received.
 */
PRIVATE void tty_do_read(TTY* tty, MESSAGE* msg) {
    // tell the tty
    tty->tty_caller = msg->source;      // who called, usually FS
    tty->tty_proc_nr = msg->PROC_NR;    // who wants the chars
    tty->tty_req_buf = va2la(tty->tty_proc_nr, msg->BUF);   // where the chars should be put
    tty->tty_left_cnt = msg->CNT;       // how many chars are requested
    tty->tty_trans_cnt = 0;             // how many chars have been transferred

    msg->type = SUSPEND_PROC;
    msg->CNT = tty->tty_left_cnt;
    send_recv(SEND, tty->tty_caller, msg);
}

/**
 * Invoked when task TTY receives DEV_WRITS message.
 *
 * @param tty   To which TTY the caller proc is bound.
 * @param mag   The MESSAGE.
 *
 */
PRIVATE void tty_do_write(TTY* tty, MESSAGE* msg) {
    char buf[TTY_OUT_BUF_LEN];
    char *p = (void *) va2la(msg->PROC_NR, msg->BUF);
    int i = msg->CNT;
    int j;

    while (i) {
        int bytes = min(TTY_OUT_BUF_LEN, i);
        phys_copy(va2la(TASK_TTY, buf), (void *)p, bytes);
        for (j = 0; j < bytes; j++) {
            out_char(tty->p_console, buf[j]);
        }
        i -= bytes;
        p += bytes;
    }
    msg->type = SYSCALL_RET;
    send_recv(SEND, msg->source, msg);
}

PUBLIC int sys_printx(int _unusedl, int _unused2, char* s, struct proc* p_proc) {
    const char * p;
    char ch;

    char reenter_err[] = "? k_reenter is incorrect for unknown reason";
    reenter_err[0] = MAG_CH_PANIC;

    if (k_reenter == 0) {
        p = va2la(proc2pid(p_proc), s);
    }else if (k_reenter > 0) {
        p = s;
    }else {
        p = reenter_err;
    }

    if ((*p == MAG_CH_PANIC) || (*p == MAG_CH_ASSERT && p_proc_ready < &proc_table[NR_TASKS])) {
        disable_int();
        char * v = (char*)V_MEM_BASE;
        const char * q = p + 1;         // + 1; skip the magic char

        while (v < (char *)(V_MEM_BASE + V_MEM_SIZE)) {
            *v++ = *q++;
            *v++ = RED_CHAR;
            if (!*q) {
                while (((int)v - V_MEM_SIZE) & (SCR_WIDTH * 16)) {
                    v++;
                    *v++ = GRAY_CHAR;
                }
                q = p + 1;
            }
        }

        __asm__ __volatile__("hlt");
    }

    while ((ch = *p++) != 0) {
        if (ch == MAG_CH_PANIC || ch == MAG_CH_ASSERT) continue; // skip the magic char
        out_char(tty_table[p_proc->nr_tty].p_console, ch);
    }

    return 0;
}
