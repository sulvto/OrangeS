//
// Created by sulvto on 17-8-23.
//

/**
 * open/create a file
 * @param pathname  The full path of the file to be opened/created.
 * @param flags     O_CREAT, O_RDWR, ect.
 */
PUBLIC int open(const char *pathname, int flags) {
    MESSAGE msg;
    msg.type = OPEN;
    
    msg.PATHNAME = (void*)pathname;
    msg.FLAGS    = flags;
    msg.NAME_LEN = strilen(pathname);
    
    send_recv(BOTH, TASK_FS, &msg);
    assert(msg.type == SYSCALL_RET);

    return msg.FD;
}
