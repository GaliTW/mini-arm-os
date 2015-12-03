#ifndef __THREADS_H__
#define __THREADS_H__

#include "os.h"

void thread_start();
int thread_create(void (*run)(int, char *[]), char *name, void *userdata);
void thread_kill(int thread_id);
void thread_self_terminal();

typedef struct {
	void *stack;
	void *orig_stack;
	void *args;
	void *argv;
	uint8_t in_use;
} tcb_t;

extern tcb_t tasks[MAX_TASKS];

#endif
