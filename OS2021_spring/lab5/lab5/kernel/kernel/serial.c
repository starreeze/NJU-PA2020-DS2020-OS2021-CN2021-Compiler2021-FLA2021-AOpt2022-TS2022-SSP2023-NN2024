#include "x86.h"

#define SERIAL_PORT  0x3F8

void initSerial(void) {
	outByte(SERIAL_PORT + 1, 0x00);
	outByte(SERIAL_PORT + 3, 0x80);
	outByte(SERIAL_PORT + 0, 0x01);
	outByte(SERIAL_PORT + 1, 0x00);
	outByte(SERIAL_PORT + 3, 0x03);
	outByte(SERIAL_PORT + 2, 0xC7);
	outByte(SERIAL_PORT + 4, 0x0B);
}

static inline int serialIdle(void) {
	return (inByte(SERIAL_PORT + 5) & 0x20) != 0;
}

void putChar(char ch) {
	while (serialIdle() != TRUE);
	outByte(SERIAL_PORT, ch);
}

void putString(const char *str) {
	int i = 0;
	if (str == NULL) {
		return;
	}
	while (str[i] != 0) {
		putChar(str[i++]);
	}
}

void putInt(int a) {
	char buf[32];
	char *p = buf + sizeof(buf) - 1;
	*p = '\0';
	*(--p) = '\n';
	do {
		*--p = '0' + a % 10;
	} while (a /= 10);
	putString(p);
}
