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
 * Read from a file descriptor.
 * @param fd File descriptor.
 * @param buf Buffer to accept the bytes read.
 * @param count Hom many bytes to read.
 * @return On success,the number of bytes read are returned.
 *         On error ,-1 is returned.
 */
PUBLIC int read(int fd, void *buf, int count) {
    MESSAGE msg;
    msg.type = READ;
    msg.FD = fd;
    msg.BUF = buf;
    msg.CNT = count;

    send_recv(BOTH, TASK_FS, &msg);

    return msg.CNT;
}
