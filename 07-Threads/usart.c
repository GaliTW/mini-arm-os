#include "os.h"
#include "reg.h"
#include "stdio.h"
#include "buffer.h"

void *usart_rx_buffer;
void *stdin_buffer;
char usart_rx_raw_buffer[BUFFER_SIZE];
char stdin_raw_buffer[BUFFER_SIZE];

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

	usart_rx_buffer = buf_init(usart_rx_raw_buffer, sizeof(char), BUFFER_SIZE);
	stdin = stdin_buffer = buf_init(stdin_raw_buffer, sizeof(char), BUFFER_SIZE);
}

void usart2_handler()
{
	char c;
	while (*(USART2_SR) & USART_FLAG_RXNE) {
		c = *USART2_DR & 0xff;
		if (c == '\r' || c == '\n') {
			buf_push_front(&c, usart_rx_buffer);
			buf_copy_front(usart_rx_buffer, stdin_buffer);
			putsln("");
		} else if (c == '\b' || c == (char)127) {
			if (buf_get_front(&c, usart_rx_buffer) == 1)
				puts("\b \b");
		} else {
			buf_push_front(&c, usart_rx_buffer);
			putchar(c);
			if (buf_full(usart_rx_buffer))
				buf_copy_front(usart_rx_buffer, stdin_buffer);
		}
	}
}
