#include <stddef.h>
#include <stdint.h>
#include "os.h"
#include "reg.h"
#include "threads.h"
#include "stdio.h"
#include "malloc.h"
#include "string.h"
#include "usart.h"
#include "buffer.h"
#include "romfs.h"

extern unsigned _fibonacci(int num);

void fibonacci(int argc, char *argv[])
{
	if (argc == 2)
		putd(_fibonacci(strtoi(argv[1])));
	else
		putd(0);

	putsln("");
}

void ls(const char *path)
{
	void *dir = opendir(path);
	if (dir == NULL) {
		putsln("WTF!?");
		return;
	}

	printdir(dir);
}

void cd(const char *to_path, char *cur_path)
{
	void *dir;
	char *path, *npath;
	if (*to_path == '/') {
		npath = path = (char *)malloc((strlen(to_path) + 1) * sizeof(char));
		normal_path(to_path, npath);
	} else {
		path = (char *)malloc((2 * (strlen(to_path) + strlen(cur_path)) + 4) * sizeof(char));
		strcpy(path, cur_path);
		strcat(path, "/");
		strcat(path, to_path);
		npath = path + strlen(path) + 1;
		normal_path(path, npath);
	}

	dir = opendir(npath);
	if (!dir) {
		puts(to_path);
		puts(": ");
		putsln("directory not found");
	} else
		strcpy(cur_path, npath);

	free(path);
}

void cat(const char *to_path, const char *cur_path)
{
	void *file;
	char *path, *npath;
	if (*to_path == '/') {
		npath = path = (char *)malloc((strlen(to_path) + 1) * sizeof(char));
		normal_path(to_path, npath);
	} else {
		path = (char *)malloc((2 * (strlen(to_path) + strlen(cur_path)) + 4) * sizeof(char));
		strcpy(path, cur_path);
		strcat(path, "/");
		strcat(path, to_path);
		npath = path + strlen(path) + 1;
		normal_path(path, npath);
	}

	file = openfile(npath);
	if (!file) {
		puts(to_path);
		puts(": ");
		putsln("file not found");
	} else
		printfile(file);

	free(path);
}

void shell()
{
	int size;
	char *name;
	char *command;
	int wait_thread = 0;
	char *tempStr = (char *) malloc(256 * sizeof(char));
	char working_directory[64] = "/";
	while (1) {
		puts("gali@gali-bed:");
		puts(working_directory);
		puts("$ ");

		getline(tempStr);

		buf_clear(stdin);

		/* skip spaces */
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
			else {
				stdin_key = wait_thread;
				while (tasks[wait_thread].in_use);
				stdin_key = 0;
			}
		} else if (size == 2 && (strncmp(name, "ls", 2) == 0)) {
			ls(working_directory);
		} else if (size == 2 && (strncmp(name, "cd", 2) == 0)) {
			cd(command, working_directory);
		} else if (size == 3 && (strncmp(name, "pwd", 3) == 0)) {
			putsln(working_directory);
		} else if (size == 3 && (strncmp(name, "cat", 3) == 0)) {
			cat(command, working_directory);
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
	fs_init(&_sromfs);

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
