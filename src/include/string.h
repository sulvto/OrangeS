//
// Created by sulvto on 17-7-29.
//
PUBLIC void* memcpy(void* p_dst, void* p_src, int size);
PUBLIC void  memset(void* p_dst, char ch, int size);
PUBLIC int strlen(void* p_str);
PUBLIC int strcmp(const char* s1, const char* s2);

#define phys_copy memcpy

