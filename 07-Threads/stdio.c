#include "os.h"
#include "reg.h"

void puts(const char *str)
{
	while (*str) {
		while (!(*(USART2_SR) & USART_FLAG_TXE));
		*(USART2_DR) = (*str & 0xFF);
		str++;
	}
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
