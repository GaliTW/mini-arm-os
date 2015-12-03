#ifndef __OS_H_
#define __OS_H_

#define MAX_TASKS 10
#define STACK_SIZE 256
#define MAX_HEAPS 4096

/* usart2 receive buffer */
#define USART2_RX_BUFFER_SIZE 512
extern char usart2_rx_buffer[USART2_RX_BUFFER_SIZE];
#define usart2_rx_buffer_start &usart2_rx_buffer[0]
#define usart2_rx_buffer_end   &usart2_rx_buffer[USART2_RX_BUFFER_SIZE]
extern char *usart2_rx_start;
extern char *usart2_rx_end;

#endif
