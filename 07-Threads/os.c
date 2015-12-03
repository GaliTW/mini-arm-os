#include <stddef.h>
#include <stdint.h>
#include "os.h"
#include "reg.h"
#include "threads.h"
#include "stdio.h"

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

static void delay(volatile int count)
{
	count *= 50000;
	while (count--);
}

static void busy_loop(void *str)
{
	while (1) {
		print_str(str);
		print_str(": Running...\n");
		delay(1000);
	}
}

void test1(void *userdata)
{
	busy_loop(userdata);
}

void test2(void *userdata)
{
	busy_loop(userdata);
}

void test3(void *userdata)
{
	busy_loop(userdata);
}

/* 72MHz */
#define CPU_CLOCK_HZ 72000000

/* 100 ms per tick. */
#define TICK_RATE_HZ 10

int main(void)
{
	const char *str1 = "Task1", *str2 = "Task2", *str3 = "Task3";

	usart_init();

	if (thread_create(test1, (void *) str1) == -1)
		print_str("Thread 1 creation failed\r\n");

	if (thread_create(test2, (void *) str2) == -1)
		print_str("Thread 2 creation failed\r\n");

	if (thread_create(test3, (void *) str3) == -1)
		print_str("Thread 3 creation failed\r\n");

	/* SysTick configuration */
	*SYSTICK_LOAD = (CPU_CLOCK_HZ / TICK_RATE_HZ) - 1UL;
	*SYSTICK_VAL = 0;
	*SYSTICK_CTRL = 0x07;

	/* USART2 interrupt configuration */
	*NVIC_ISER1 = 1 << 6 ;// USART2 = 38 = 31 + 7
	thread_start();

	return 0;
}
