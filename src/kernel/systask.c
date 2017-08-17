//
// Created by sulvto on 17-8-17.
//

/**
 * <Ring 1> The main loop of TASK SYS.
 * 
 */
PUBLIC void task_sys() {
    MESSAGE msg;
    while (1) {
        send_recv(RECEIVE, ANY, &msg);
        int src = msg.source;

        switch (msg.type) {
            case GET_TICKS:
                msg.RETVAL = ticks;
                send_recv(SEND, src, &msg);
                break;
            default:
                panic("unknown msg type");
                break;
        }
    }
}

