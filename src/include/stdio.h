//
// Created by sulvto on 17-8-26.
//


#define ASSERT
#ifdef ASSERT
void assertion_failure(char *exp, char *file, char *base_file, int line);
#define assert(exp) if (exp) ; else assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)
#else 
#define assert(exp)
#endif


#define EXTERN extern

// string
#define STR_DEFAULT_LEN 1024


#define O_CREAT     1
#define O_RDWR      2


/**
 * 库函数
 */

// lib/open.c
PUBLIC int open(const char *pathname, int flags);

// lib/close.c
PUBLIC int close(int fd);

// lib/read.c
PUBLIC int read(int fd, void *buf, int count);

// lib/write.c
PUBLIC int write(int fd, const void *buf, int count);
