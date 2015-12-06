#include <stddef.h>
#include <stdint.h>
#include "os.h"
#include "reg.h"
#include "threads.h"
#include "stdio.h"
#include "malloc.h"
#include "string.h"
#include "usart.h"

extern unsigned _fibonacci(int num);

void fibonacci(int argc, char *argv[])
{
	char rst[11];
	if (argc == 2) {
		itostr(_fibonacci(strtoi(argv[1])), rst);
		puts(rst);
		puts("\r\n");
	}
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
