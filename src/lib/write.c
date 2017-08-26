//
// Created by sulvto on 17-8-26.
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

/**
 * Write to a file descriptor.
 * @param fd File descriptor.
 * @param buf Buffer including the bytes to write.
 * @param count Hom many bytes to write.
 * @return On success,the number of bytes written are returned.
 *         On error ,-1 is returned.
 */
PUBLIC int write(int fd, const void *buf, int count) {
    MESSAGE msg;
    msg.type = WRITE;
    msg.FD = fd;
    msg.BUF = (void*)buf;
    msg.CNT = count;

    send_recv(BOTH, TASK_FS, &msg);

    return msg.CNT;
}
