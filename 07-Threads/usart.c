#include "os.h"
#include "reg.h"
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
	while (*(USART2_SR) & USART_FLAG_RXNE) {
		str[0] = *USART2_DR & 0xff;
		if (str[0] == '\r') {
			str[1] = '\n';
			str[2] = '\0';
		} else
			str[1] = '\0';

		puts(str);
		*usart2_rx_start = str[0];
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
