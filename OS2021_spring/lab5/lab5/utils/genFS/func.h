#ifndef __FUNC_H__
#define __FUNC_H__

int format (const char *driver, int sectorNum, int sectorsPerBlock);

int mkdir (const char *driver, const char *destDirPath);

int rmdir (const char *driver, const char *destDirPath);

int cp (const char *driver, const char *srcFilePath, const char *destFilePath);

int rm (const char *driver, const char *destFilePath);

int ls (const char *driver, const char *destFilePath);

int touch (const char *driver, const char *destFilePath);

#endif
