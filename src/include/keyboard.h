//
// Created by sulvto on 17-8-11.
// keyboard.h

#ifndef	_ORANGES_KEYBOARD_H_
#define	_ORANGES_KEYBOARD_H_

#define KB_IN_BYTES 32      // size of keyboard input buffer

typedef struct s_kb {
    char*   p_head;         // 缓冲区下一个空闲位置
    char*   p_tail;         // 
    int     count;          // 缓冲区字节数
    char buf[KB_IN_BYTES];  // 缓冲区
}KB_INPUT;

#endif
