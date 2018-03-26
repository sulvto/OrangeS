//
// Created by sulvto on 18-3-26.
//

#include    "type.h"
#include    "stdio.h"
#include    "const.h"
#include    "protect.h"
#include    "string.h"
#include    "fs.h"
#include    "proc.h"
#include    "tty.h"
#include    "console.h"
#include    "global.h"
#include    "proto.h"

PRIVATE int fs_fork();
PUBLIC int init_mm();

/**
 * <Ring 1> The main loop of TASK MM.
 *
 */
PUBLIC void task_mm() {
	init_mm();

	while (1) {
		send_recv(RECEIVE, ANY, &mm_msg);
		int src = mm_msg.source;
		int reply = 1;

		int msgtype = mm_msg.type;

		switch (msgtype) {
			case FORK:
				mm_msg.RETVAL = do_fork();
				break;
			default:
				dump_msg("MM::unknown msg", &mm_msg);
				assert(0);
				break;
		}

		if (reply) {
			mm_msg.type = SYSCALL_RET;
			send_recv(SEND, src, &mm_msg);
		}
	}
}

/**
 *
 * Do some initialization work.
 *
 *
 */
PUBLIC int init_mm() {
	struct boot_params bp;
	get_kernel_map(&bp);

	memory_size = bp.mem_size;

	printl("(MM) memsize:%dMB\n", memory_size / (1024 * 1024));
}


/**
 * Allocate a memory block for a proc.
 *
 * @param pid 		Which proc the memroy is for.
 * @param memsize	How many bytes is needed.
 *
 * @return 	The base of the memory just allocated.
 */
PUBLIC int alloc_mem(int pid, int memsize) {
	assert(pid >= (NR_TASKS + NR_NATIVE_PROCS));

	if (memsize > PROC_IMAGE_SIZE_DEFAULT) {
		panic("unsupported memory request: %d (should be less then %d)",
						memsize,
						PROC_IMAGE_SIZE_DEFAULT);
	}

	int base = PROC_BASE + (pid - (NR_TASKS + NR_NATIVE_PROCS)) * PROC_IMAGE_SIZE_DEFAULT;

	if (base + memsize >= memory_size)
			panic("memroy allocation failed. pid:%d", pid);

	return base;
}

