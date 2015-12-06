#include "os.h"
#include "reg.h"
#include "string.h"

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
		if (usart2_rx_end != usart2_rx_start) {
			asm volatile("push {r0-r1}\n");
			asm volatile("mov r0, %0\n" : : "r"(1));
			asm volatile("mov r1, %0\n" : : "r"(strPtr));
			asm volatile("svc 0\n");
			asm volatile("pop {r0-r1}\n");

			if (*strPtr == '\r' || *strPtr == '\n') {
				*strPtr = '\0';
				break;
			}
			strPtr++;
			i++;
		}
	}
	return i;
}
