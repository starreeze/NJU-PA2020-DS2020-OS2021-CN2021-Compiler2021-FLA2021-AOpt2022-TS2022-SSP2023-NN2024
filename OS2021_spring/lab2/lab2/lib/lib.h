#ifndef __lib_h__
#define __lib_h__

#define SYS_WRITE 0 // 可修改 可自定义
#define STD_OUT 0   // 可修改 可自定义

#define SYS_READ 1 // 可修改 可自定义
#define STD_IN 0   // 可修改 可自定义
#define STD_STR 1  // 可修改 可自定义

#define MAX_BUFFER_SIZE 256

void printf(const char *format, ...);
char getChar();
void getStr(char *str, int size);

#endif
