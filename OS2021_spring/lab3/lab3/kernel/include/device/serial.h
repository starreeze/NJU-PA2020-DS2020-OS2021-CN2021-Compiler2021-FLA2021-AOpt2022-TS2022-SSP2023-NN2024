#ifndef __SERIAL_H__
#define __SERIAL_H__

void initSerial(void);
void putChar(char);
// output log (no more than 100 char)
void log(const char *str);
void logint(uint32_t num);
#define SERIAL_PORT 0x3F8

#endif
