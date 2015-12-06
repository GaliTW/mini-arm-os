#ifndef __GSTDIO_H_
#define __GSTDIO_H_

void putchar(char c);
void puts(const char *str);
void putsln(const char *str);
void putd(int value);
unsigned getline(char *strPtr);

extern void *stdin;

#endif
