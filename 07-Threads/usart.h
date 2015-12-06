#ifndef __USART_H_
#define __USART_H_

void usart_init(void);
void usart2_handler();
void svc_handler(unsigned state, void *ptr);

extern void *stdin_buffer;
#endif
