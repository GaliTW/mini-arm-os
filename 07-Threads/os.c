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
	if (argc == 2) 
		putd(_fibonacci(strtoi(argv[1])));
    else
        putd(0);

	putsln("");
}

void shell()
{
    int size;
    char *name;
    char *command;
	int wait_thread = 0;
	char *tempStr = (char *) malloc(256 * sizeof(char));
	while (1) {
		puts("gali@gali-bed:/$ ");
		getline(tempStr);
        while (*tempStr) {
            if (*tempStr == ' ')
                ++tempStr;
            else
                break;
        }

        command = tempStr;
        name = strsep(&command, " ");
        size = strlen(name);

        if (size == 0)
            continue;
		if (size == 3 && (strncmp(name, "fib", 3) == 0)) {
			if ((wait_thread = thread_create(fibonacci, name, (void *) command)) == -1)
				putsln("fibonacci thread creation failed");
			else
				while (tasks[wait_thread].in_use);
		} else
			putsln("Command not found!");
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
		putsln("shell thread creation failed");

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
