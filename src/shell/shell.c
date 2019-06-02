//
// Created by sulvto on 18-11-26.
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
#include "buildin_cmd.h"

int streq(const char *src, const char *dest) {
	if (strlen(src) == strlen(dest) && strcmp(src, dest) == 0) {
		return 0;
	} else {
		return -1;
	}
}

// 提示符
PRIVATE void print_prompt(void) {
    printl("[sulvto@localhost ~]$ ");
}

PRIVATE void readline(char *buf, u32 count) {
	read(STDIN_NO, buf, count);
}

// 解析 提取参数个数
PRIVATE u32 cmd_parse(char *cmd_str, char **argv, char token) {
	
	u32 arg_index = 0;
    while (arg_index < MAX_ARG_NR) {
   		argv[arg_index] = 0;
		arg_index++;
    }	

	char *next = cmd_str;
	u32 argc = 0;
	while (*next) {
		// skip token
		while (*next == token) {
			next++;
		}
		if (*next == 0) {
			break;
		}

		argv[argc] = next;

		while (*next && *next != token) {
			next++;
		}

		if (*next) {
			*next++ = '\0';
		}

		if (argc > MAX_ARG_NR) {
			return -1;
		}

		argc++;
	}

	return argc;
}

PRIVATE void cmd_execute(u32 argc, char **argv) {
    if (streq("help", argv[0]) == 0) {
		buildin_help(argc, argv);
	} else if (streq("ls", argv[0]) == 0) {
		buildin_ls(argc, argv);
	} else if (streq("pwd", argv[0]) == 0) {
		buildin_pwd(argc, argv);
    }  else {
        // TODO: pwd ps clear mkdir

        // No such file or directory
        // execv()
    }
}


PRIVATE char *argv[MAX_ARG_NR] = {};
PRIVATE u32 argc = -1;

PRIVATE char cmd_line[MAX_PATH_LEN] = {0};

// 简单的shell

/**
 * 
 *  A very very simple shell.
 * @param tty_name TTY file name
 * 
 */ 
PUBLIC void my_shell(const char *tty_name) {

    int fd_stdin = open(tty_name, O_RDWR);
    assert(fd_stdin == STDIN_NO);
    int fd_stdout = open(tty_name, O_RDWR);
    assert(fd_stdout == STDOUT_NO);

    while (1) {
		memset(cmd_line, 0, MAX_PATH_LEN);
        print_prompt();
        readline(cmd_line, MAX_PATH_LEN);
		if (cmd_line[0] == 0) {
			continue;
		}
        argc = -1;
        argc = cmd_parse(cmd_line, argv, ' ');
        if (argc == -1) {
            printl("num of arguments exceed %d\n", MAX_ARG_NR);
            continue;
        }
        cmd_execute(argc, argv);
    }
}





