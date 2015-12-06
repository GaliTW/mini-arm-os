#include "os.h"
#include "reg.h"
#include "string.h"
#include "buffer.h"

void *stdin;

void putchar(char c)
{
	while (!(*(USART2_SR) & USART_FLAG_TXE));
	*(USART2_DR) = (c & 0xFF);
}

void puts(const char *str)
{
	while (*str) {
		while (!(*(USART2_SR) & USART_FLAG_TXE));
		*(USART2_DR) = (*str & 0xFF);
		str++;
	}
}

void putsln(const char *str)
{
	while (*str) {
		while (!(*(USART2_SR) & USART_FLAG_TXE));
		*(USART2_DR) = (*str & 0xFF);
		str++;
	}
	putchar('\r');
	putchar('\n');
}

void putd(int value)
{
	char tmp[12];
	itostr(value, tmp);
	puts(tmp);
}

unsigned getline(char *strPtr)
{
	unsigned i = 0;
	while (1) {
		if (buf_get_back(strPtr, stdin) == 1) {
			if (*strPtr == '\n' || *strPtr == '\r') {
				if (*strPtr == '\r' && *buf_seek_back(stdin) == '\n')
					buf_get_back(strPtr, stdin);
				*strPtr = '\0';
				break;
			}
			++strPtr;
			++i;
		}
	}
	return i;
}
