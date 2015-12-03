#include <stddef.h>
#include <stdint.h>
#include "os.h"
#include "reg.h"
#include "threads.h"
#include "stdio.h"
#include "malloc.h"
#include "string.h"

char usart2_rx_buffer[USART2_RX_BUFFER_SIZE];
char *usart2_rx_start = usart2_rx_buffer_start;
char *usart2_rx_end = usart2_rx_buffer_start;

void usart_init(void)
{
	*(RCC_APB2ENR) |= (uint32_t)(0x00000001 | 0x00000004);
	*(RCC_APB1ENR) |= (uint32_t)(0x00020000);

	/* USART2 Configuration, Rx->PA3, Tx->PA2 */
	*(GPIOA_CRL) = 0x00004B00;
	*(GPIOA_CRH) = 0x44444444;
	*(GPIOA_ODR) = 0x00000000;
	*(GPIOA_BSRR) = 0x00000000;
	*(GPIOA_BRR) = 0x00000000;

	*(USART2_CR1) = 0x0000000C;
	*(USART2_CR2) = 0x00000000;
	*(USART2_CR3) = 0x00000000;
	*(USART2_CR1) |= 0x2000;
	*(USART2_CR1) |= 0x20;
}

void usart2_handler()
{
	char str[3];
	char c;
	while (*(USART2_SR) & USART_FLAG_RXNE) {
		c = str[0] = *USART2_DR & 0xff;
		if (str[0] == '\r') {
			str[0] = '\r';
			str[1] = '\n';
			str[2] = '\0';
		} else
			str[1] = '\0';

		puts(str);
		*usart2_rx_start = c;
		++usart2_rx_start;
		if (usart2_rx_start == usart2_rx_buffer_end)
			usart2_rx_start = usart2_rx_buffer_start;

		if (usart2_rx_start == usart2_rx_end) {
			++usart2_rx_end;
			if (usart2_rx_end == usart2_rx_buffer_end)
				usart2_rx_end = usart2_rx_buffer_start;
		}
	}

}

void svc_handler(unsigned state, void *ptr)
{
	if (state == 1) {
		char *str = (char *) ptr;
		*str = *usart2_rx_end;
		++usart2_rx_end;
		if (usart2_rx_end == usart2_rx_buffer_end)
			usart2_rx_end = usart2_rx_buffer_start;
	}
}

void print_str(const char *str)
{
	while (*str) {
		while (!(*(USART2_SR) & USART_FLAG_TXE));
		*(USART2_DR) = (*str & 0xFF);
		str++;
	}
}

void fibonacci(int argc, char *argv[])
{
	puts(argv[0]);
	puts("\r\n");
}

void shell()
{
	int wait_thread = 0;
	char *tempStr = (char *) malloc(1024 * sizeof(char));
	while (1) {
		puts("gali@gali-bed:/$ ");
		getline(tempStr);
		char *command = tempStr;
		char *name = strsep(&command, " ");

		if ((strlen(name) == 3) && (strncmp(name, "fib", 3) == 0)) {
			if ((wait_thread = thread_create(fibonacci, name, (void *) command)) == -1)
				puts("fibonacci thread creation failed\r\n");
			else
				while (tasks[wait_thread].in_use);
		} else
			puts("Command not found!\r\n");
	}
	while (1);
}

/* 72MHz */
#define CPU_CLOCK_HZ 72000000

/* 100 ms per tick. */
#define TICK_RATE_HZ 10

int main(void)
{
	usart_init();

	if (thread_create(shell, "shell", NULL) == -1)
		puts("shell thread creation failed\r\n");

	/* SysTick configuration */
	*SYSTICK_LOAD = (CPU_CLOCK_HZ / TICK_RATE_HZ) - 1UL;
	*SYSTICK_VAL = 0;
	*SYSTICK_CTRL = 0x07;

	/* USART2 interrupt configuration */
	*NVIC_ISER1 = 1 << 6 ;// USART2 = 38 = 31 + 7

	thread_start();

	while (1);
	return 0;
}
