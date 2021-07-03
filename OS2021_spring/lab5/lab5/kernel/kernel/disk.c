#include "x86.h"
#include "device.h"

void waitDisk(void) {
	while((inByte(0x1F7) & 0xC0) != 0x40); 
}

void readSect(void *dst, int offset) {
	int i;
	waitDisk();
	
	outByte(0x1F2, 1);
	outByte(0x1F3, offset);
	outByte(0x1F4, offset >> 8);
	outByte(0x1F5, offset >> 16);
	outByte(0x1F6, (offset >> 24) | 0xE0);
	outByte(0x1F7, 0x20);

	waitDisk();
	for (i = 0; i < SECTOR_SIZE / 4; i ++) {
		((uint32_t *)dst)[i] = inLong(0x1F0);
	}
}

void writeSect(void *src, int offset) {
	int i;
	waitDisk();

	outByte(0x1F2, 1);
	outByte(0x1F3, offset);
	outByte(0x1F4, offset >> 8);
	outByte(0x1F5, offset >> 16);
	outByte(0x1F6, (offset >> 24) | 0xE0);
	outByte(0x1F7, 0x30);

	waitDisk();
	for (i = 0; i < SECTOR_SIZE / 4; i ++) {
		outLong(0x1F0, ((uint32_t *)src)[i]);
	}
}

void diskRead (void *destBuffer, int size, int num, int offset) {
	int i = 0;
	int j = 0;
	uint8_t buffer[SECTOR_SIZE];
	int quotient = offset / SECTOR_SIZE;
	int remainder = offset % SECTOR_SIZE;
	
	readSect((void*)buffer, 201 + quotient + j);
	j ++;
	while (i < size * num) {
		((uint8_t*)destBuffer)[i] = buffer[(remainder + i) % SECTOR_SIZE];
		i ++;
		if ((remainder + i) % SECTOR_SIZE == 0) {
			readSect((void*)buffer, 201 + quotient + j);
			j ++;
		}
	}
}

void diskWrite (void *destBuffer, int size, int num, int offset) {
	int i = 0;
	int j = 0;
	uint8_t buffer[SECTOR_SIZE];
	int quotient = offset / SECTOR_SIZE;
	int remainder = offset % SECTOR_SIZE;

	readSect((void*)buffer, 201 + quotient + j);
	while (i < size * num) {
		buffer[(remainder + i) % SECTOR_SIZE] = ((uint8_t*)destBuffer)[i];
		i ++;
		if ((remainder + i) % SECTOR_SIZE == 0) {
			writeSect((void*)buffer, 201 + quotient + j);
			j ++;
			readSect((void*)buffer, 201 + quotient + j);
		}
	}
	writeSect((void*)buffer, 201 + quotient + j);
}
