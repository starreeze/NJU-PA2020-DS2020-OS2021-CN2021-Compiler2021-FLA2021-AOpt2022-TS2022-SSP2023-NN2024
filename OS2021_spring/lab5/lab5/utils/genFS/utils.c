#include <stdlib.h>
#include "utils.h"

/*
* find the first token in string
* set *size as the number of bytes before token
* if not found, set *size as the length of *string
*/
int stringChr (const char *string, char token, int *size) {
	int i = 0;
	if (string == NULL) {
		*size = 0;
		return -1;
	}
	while (string[i] != 0) {
		if (token == string[i]) {
			*size = i;
			return 0;
		}
		else
			i ++;
	}
	*size = i;
	return -1;
}

/*
* find the last token in string
* set *size as the number of bytes before token
* if not found, set *size as the length of *string
*/
int stringChrR (const char *string, char token, int *size) {
	int i = 0;
	if (string == NULL) {
		*size = 0;
		return -1;
	}
	while (string[i] != 0)
		i ++;
	*size = i;
	while (i > -1) {
		if (token == string[i]) {
			*size = i;
			return 0;
		}
		else
			i --;
	}
	return -1;
}

int stringLen (const char *string) {
	int i = 0;
	if (string == NULL)
		return 0;
	while (string[i] != 0)
		i ++;
	return i;
}

int stringCmp (const char *srcString, const char *destString, int size) { // compre first 'size' bytes
	int i = 0;
	if (srcString == NULL || destString == NULL)
		return -1;
	while (i != size) {
		if (srcString[i] != destString[i])
			return -1;
		else if (srcString[i] == 0)
			return 0;
		else
			i ++;
	}
	return 0;
}

int stringCpy (const char *srcString, char *destString, int size) {
	int i = 0;
	if (srcString == NULL || destString == NULL)
		return -1;
	while (i != size) {
		if (srcString[i] != 0) {
			destString[i] = srcString[i];
			i++;
		}
		else
			break;
	}
	destString[i] = 0;
	return 0;
}

int setBuffer (uint8_t *buffer, int size, uint8_t value) {
	int i = 0;
	if (buffer == NULL)
		return -1;
	for (i = 0; i < size ; i ++)
		buffer[i] = value;
	return 0;
}
