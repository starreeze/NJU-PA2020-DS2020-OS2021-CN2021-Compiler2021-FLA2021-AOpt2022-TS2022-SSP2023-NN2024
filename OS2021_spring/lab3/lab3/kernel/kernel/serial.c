#include "x86.h"
#include "device.h"

void initSerial(void)
{
	outByte(SERIAL_PORT + 1, 0x00);
	outByte(SERIAL_PORT + 3, 0x80);
	outByte(SERIAL_PORT + 0, 0x01);
	outByte(SERIAL_PORT + 1, 0x00);
	outByte(SERIAL_PORT + 3, 0x03);
	outByte(SERIAL_PORT + 2, 0xC7);
	outByte(SERIAL_PORT + 4, 0x0B);
}

static inline int serialIdle(void)
{
	return (inByte(SERIAL_PORT + 5) & 0x20) != 0;
}

void putChar(char ch)
{
	while (serialIdle() != TRUE)
		;
	outByte(SERIAL_PORT, ch);
}

void log(const char *str)
{
	for (int i = 0; i < 100 && str[i]; ++i)
		putChar(str[i]);
}

void logint(uint32_t num)
{
	if (!num)
		putChar('0');
	char buf[12];
	int i = 0;
	for (; num; ++i)
	{
		buf[i] = (char)(num % 10 + '0');
		num /= 10;
	}
	while (--i >= 0)
		putChar(buf[i]);
}