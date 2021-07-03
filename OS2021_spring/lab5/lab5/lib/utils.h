#ifndef __UTILS_H__
#define __UTILS_H__

int stringChr(const char *string, char token, int *size);

int stringChrR (const char *string, char token, int *size);

int stringLen(const char *string);

int stringCmp(const char *srcString, const char *destString, int size);

int stringCpy (const char *srcString, char *destString, int size);

int setBuffer (uint8_t *buffer, int size, uint8_t value);

#endif
