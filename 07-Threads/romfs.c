#include <stddef.h>
#include <stdint.h>
#include "romfs.h"
#include "malloc.h"
#include "stdio.h"
#include "string.h"

typedef struct __DIRECTORY_STRUCT {
	uint8_t type;
	uint32_t parent;
	uint32_t sibling;
	uint32_t children;
	char name;
} __attribute__((packed)) dir_t;

typedef struct __FILE_STRUCT {
	uint8_t type;
	uint32_t parent;
	uint32_t sibling;
	uint32_t size;
	char name;
} __attribute__((packed)) file_t;

typedef struct __ROMFS_STRUCT {
	uint32_t magic;
	dir_t root;
} __attribute__((packed)) romfs_t;

#define member_to_entry(ptr, type, member) \
    ((type *)((char *)fs + (ptr) - (unsigned long)(&((type *)0)->member)))

#define next(x, type) \
    (*(type *)((char *)fs + (x)))

static romfs_t *fs = NULL;

unsigned char fs_init(unsigned char *addr)
{
	if (fs != NULL || addr == NULL)
		return 0;

	if (((romfs_t *)((void *)addr))->magic != 0x006AF500)
		return 0;

	fs = (romfs_t *)((void *)addr);
	return 1;
}

void printfile(void *filentry)
{
	uint32_t size = ((file_t *)filentry)->size;
	char *name = &((file_t *)filentry)->name;
	char *c = name + strlen(name) + 1;
	while (size-- > 0)
		putchar(*c++);
}

void printdir(void *direntry)
{
	uint32_t head, node;
	unsigned char hasNode = 0;

	if (!fs || ((dir_t *)direntry)->type != 0)
		return;

	head = ((dir_t *)direntry)->children;
	for (node = head; next(node, uint32_t) != head; node = next(node, uint32_t)) {
		puts(&(member_to_entry(node, dir_t, sibling)->name));
		if (member_to_entry(node, dir_t, sibling)->type == 0)
			putchar('/');
		puts("\t\t");
		hasNode = 1;
	}

	if (hasNode)
		putsln("");
}

void *findentry(const char *path, uint8_t type)
{
	const char *s, *e;
	uint32_t head, node;
	dir_t *entry;
	unsigned match = 1;

	if (!fs)
		return NULL;

	if (*(++path) == '\0')
		return &fs->root;

	e = s = path;

	head = fs->root.children;
	while (match) {
		match = 0;
		while (*e != '\0' && *e != '/')
			++e;

		for (node = head; next(node, uint32_t) != head; node = next(node, uint32_t)) {
			entry = member_to_entry(node, dir_t, sibling);

			if (*e == '/') {
				if (entry->type == 0 &&
				    strlen(&entry->name) == (e - s) &&
				    strncmp(&entry->name, s, (e - s)) == 0) {
					head = entry->children;
					match = 1;
					break;
				}
			} else {
				if (entry->type == type &&
				    strlen(&entry->name) == (e - s) &&
				    strncmp(&entry->name, s, (e - s)) == 0) {
					return entry;
				}
			}
		}

		if (*e == '\0')
			return NULL;

		s = ++e;
	}
	return NULL;
}

void *openfile(const char* file)
{
	return findentry(file, 1);
}

void *opendir(const char* directory)
{
	return findentry(directory, 0);
}

void normal_path(const char *path, char *npath)
{
	char *head = npath;
	char pre = 0, dot = 0;

	if (*path == '\\' || *path == '/') {
		++path;
		pre = 1;
	}

	*npath = '/';

	while (*path) {
		if (*path == '\\' || *path == '/') {
			if (dot == 1) {
				--npath;
			} else if (dot == 2) {
				npath -= 2;
				while (npath != head)
					if (*--npath == '/')
						break;
			} else if (pre != 1)
				*++npath = '/';
			pre = 1;
			dot = 0;
		} else if (*path == '.') {
			*++npath = '.';
			++dot;
			pre = 0;
		} else {
			*++npath = *path;
			pre = 0;
			dot = 0;
		}

		++path;
	}

	if (dot == 1) {
		--npath;
	} else if (dot == 2) {
		npath -= 2;
		while (npath != head)
			if (*--npath == '/')
				break;
	}

	if (*npath == '/' && npath != head)
		*npath = '\0';
	else
		*++npath = '\0';
}

#undef member_to_entry
#undef next
