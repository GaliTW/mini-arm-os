#include <stdint.h>
#include "threads.h"
#include "os.h"
#include "malloc.h"
#include "reg.h"
#include "string.h"

#define THREAD_PSP	0xFFFFFFFD

/* Thread Control Block */
tcb_t tasks[MAX_TASKS];
static int lastTask;
static int first = 1;

/* FIXME: Without naked attribute, GCC will corrupt r7 which is used for stack
 * pointer. If so, after restoring the tasks' context, we will get wrong stack
 * pointer.
 */
void __attribute__((naked)) pendsv_handler()
{
	/* Save the old task's context */
	asm volatile("mrs   r0, psp\n"
	             "stmdb r0!, {r4-r11, lr}\n");
	/* To get the task pointer address from result r0 */
	asm volatile("mov   %0, r0\n" : "=r"(tasks[lastTask].stack));

	/* Find a new task to run */
	while (1) {
		lastTask++;
		if (lastTask == MAX_TASKS)
			lastTask = 0;
		if (tasks[lastTask].in_use) {
			/* Move the task's stack pointer address into r0 */
			asm volatile("mov r0, %0\n" : : "r"(tasks[lastTask].stack));
			/* Restore the new task's context and jump to the task */
			asm volatile("ldmia r0!, {r4-r11, lr}\n"
			             "msr psp, r0\n"
			             "bx lr\n");
		}
	}
}

void systick_handler()
{
	*SCB_ICSR |= SCB_ICSR_PENDSVSET;
}

void thread_start()
{
	lastTask = 0;

	/* Save kernel context */
	asm volatile("mrs ip, psr\n"
	             "push {r4-r11, ip, lr}\n");

	/* To bridge the variable in C and the register in ASM,
	 * move the task's stack pointer address into r0.
	 * http://www.ethernut.de/en/documents/arm-inline-asm.html
	 */
	asm volatile("mov r0, %0\n" : : "r"(tasks[lastTask].stack));
	/* Load user task's context and jump to the task */
	asm volatile("msr psp, r0\n"
	             "mov r0, #3\n"
	             "msr control, r0\n"
	             "isb\n"
	             "pop {r4-r11, lr}\n"
	             "pop {r0}\n"
	             "bx lr\n");
}

int thread_create(void (*run)(int, char *[]), char *name, void *userdata)
{
	int threadId = 0;
	uint32_t *stack;

	char *args = (char *)userdata;

	int args_size = strlen(args) + 1;
	int name_size = strlen(name) + 1;

	int argc = 1;
	char *args_buffer;
	char **argv_buffer;

	/* Find a free thing */
	for (threadId = 0; threadId < MAX_TASKS; threadId++) {
		if (tasks[threadId].in_use == 0)
			break;
	}

	if (threadId == MAX_TASKS)
		return -1;

	stack = (uint32_t *) malloc((STACK_SIZE) * sizeof(uint32_t) + (args_size + name_size) * sizeof(char) + 32 * sizeof(char *));
	if (stack == 0)
		return -1;

	/* Create function arguments */
	args_buffer = (char *)(stack + STACK_SIZE);
	argv_buffer = (char **)(args_buffer + (args_size + name_size));

	strcpy(args_buffer, name);
	strcpy(args_buffer + name_size, args);
	args = args_buffer + name_size;

	argv_buffer[0] = args_buffer;
	while (args != NULL && ((argv_buffer[argc] = strsep(&args, " ")) != NULL))
        if (strlen(argv_buffer[argc]) > 0)
    		argc++;

	/* Create the stack */
	tasks[threadId].orig_stack = stack;

	stack += STACK_SIZE - 17; /* End of stack, minus what we are about to push */
	if (first) {
		stack[8] = (unsigned int) run;
		stack[9] = (unsigned int) argc;
		stack[10] = (unsigned int) argv_buffer;
		first = 0;
	} else {
		stack[8] = (unsigned int) THREAD_PSP;
		stack[9] = (unsigned int) argc;
		stack[10] = (unsigned int) argv_buffer;
		stack[14] = (unsigned) &thread_self_terminal;
		stack[15] = (unsigned int) run;
		stack[16] = (unsigned int) 0x21000000; /* PSR Thumb bit */
	}

	/* Construct the control block */
	tasks[threadId].stack = stack;
	tasks[threadId].args = args_buffer;
	tasks[threadId].argv = argv_buffer;
	tasks[threadId].in_use = 1;

	return threadId;
}

void thread_kill(int thread_id)
{
	/* Free the stack */
	free(tasks[thread_id].orig_stack);

	tasks[thread_id].in_use = 0;
}

void thread_self_terminal()
{
	/* This will kill the stack.
	 * For now, disable context switches to save ourselves.
	 */
	asm volatile("cpsid i\n");
	thread_kill(lastTask);
	asm volatile("cpsie i\n");

	/* after kill, enforce to context switch */
	*SCB_ICSR |= SCB_ICSR_PENDSVSET;
}
