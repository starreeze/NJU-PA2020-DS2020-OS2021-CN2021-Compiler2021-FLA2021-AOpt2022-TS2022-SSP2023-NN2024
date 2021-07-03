#ifndef __DISK_H__
#define __DISK_H__

#define SECTOR_NUM 8196

#define SECTOR_SIZE 512

void waitDisk(void);
void readSect(void *dst, int offset);
void writeSect(void *src, int offset);

void diskRead (void *destBuffer, int size, int num, int offset);
void diskWrite (void *destBuffer, int size, int num, int offset);

#endif
