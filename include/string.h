#ifndef __STRING_H__
#define __STRING_H__

#include "common.h"

char *itoa(int);
void memcpy(void *, const void *, size_t);
void memset(void *, uint8_t, size_t);
size_t strlen(const char *);
void strcpy(char *, const char *);
int strcmp(const char *, const char *);

#endif
