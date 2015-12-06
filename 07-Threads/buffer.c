#include <stddef.h>
#include "buffer.h"
#include "malloc.h"

typedef struct __BUFFER_STRUCT {
	char *head;
	char *tail;
	char *front;
	char *back;
	unsigned int t_size;
} Buffer_t;

static inline char *incptr(char *p, void *buf)
{
	if (p + ((Buffer_t *)buf)->t_size == ((Buffer_t *)buf)->tail)
		return ((Buffer_t *)buf)->head;
	return p + ((Buffer_t *)buf)->t_size;
}

static inline char *decptr(char *p, void *buf)
{
	if (p == ((Buffer_t *)buf)->head)
		return ((Buffer_t *)buf)->tail - ((Buffer_t *)buf)->t_size;
	return p - ((Buffer_t *)buf)->t_size;
}

void *buf_init(char *raw_buf, unsigned int type_size, unsigned int total_size)
{
	Buffer_t *buf = (Buffer_t *) malloc(sizeof(Buffer_t));
	buf->front = buf->back = buf->head = raw_buf;
	buf->tail = raw_buf + (total_size * type_size);
	buf->t_size = type_size;
	return (void *)buf;
}

void buf_release(void *buf)
{
	if (buf)
		free(buf);
}

void buf_clear(void *buf)
{
	if (buf)
		((Buffer_t *)buf)->back = ((Buffer_t *)buf)->front;
}

unsigned int buf_size(void *buf)
{
	if (!buf)
		return 0;

	if (((Buffer_t *)buf)->front >= ((Buffer_t *)buf)->back)
		return (((Buffer_t *)buf)->front - ((Buffer_t *)buf)->back) / ((Buffer_t *)buf)->t_size;
	else
		return (((Buffer_t *)buf)->tail - ((Buffer_t *)buf)->back + ((Buffer_t *)buf)->front - ((Buffer_t *)buf)->head) / ((Buffer_t *)buf)->t_size;
}

unsigned int buf_total_size(void *buf)
{
	if (buf)
		return (((Buffer_t *)buf)->tail - ((Buffer_t *)buf)->head) / ((Buffer_t *)buf)->t_size;
	return 0;
}

unsigned char buf_empty(void *buf)
{
	if (buf && ((Buffer_t *)buf)->front == ((Buffer_t *)buf)->back)
		return 1;
	return 0;
}

unsigned char buf_full(void *buf)
{
	if (buf && incptr(((Buffer_t *)buf)->front, buf) == ((Buffer_t *)buf)->back)
		return 1;
	return 0;
}

unsigned char buf_get_front(char *e, void *buf)
{
	if (!buf || buf_empty(buf))
		return 0;

	((Buffer_t *)buf)->front = decptr(((Buffer_t *)buf)->front, buf);
	for (unsigned i = 0; i < ((Buffer_t *)buf)->t_size; ++i)
		*(e + i) = *(((Buffer_t *)buf)->front + i);
	return 1;
}

unsigned char buf_get_back(char *e, void *buf)
{
	if (!buf || buf_empty(buf))
		return 0;

	for (unsigned i = 0; i < ((Buffer_t *)buf)->t_size; ++i)
		*(e + i) = *(((Buffer_t *)buf)->back + i);
	((Buffer_t *)buf)->back = incptr(((Buffer_t *)buf)->back, buf);
	return 1;
}

unsigned char buf_push_front(char *e, void *buf)
{
	if (!buf || buf_full(buf))
		return 0;

	for (unsigned i = 0; i < ((Buffer_t *)buf)->t_size; ++i)
		*(((Buffer_t *)buf)->front + i) = *(e + i);
	((Buffer_t *)buf)->front = incptr(((Buffer_t *)buf)->front, buf);
	return 1;
}

unsigned char buf_push_back(char *e, void *buf)
{
	if (!buf || buf_full(buf))
		return 0;

	((Buffer_t *)buf)->back = decptr(((Buffer_t *)buf)->back, buf);
	for (unsigned i = 0; i < ((Buffer_t *)buf)->t_size; ++i)
		*(((Buffer_t *)buf)->back + i) = *(e + i);
	return 1;
}

char *buf_seek_front(void *buf)
{
	if (!buf || buf_empty(buf))
		return NULL;

	return decptr(((Buffer_t *)buf)->front, buf);
}

char *buf_seek_back(void *buf)
{
	if (!buf || buf_empty(buf))
		return NULL;

	return ((Buffer_t *)buf)->back;
}

unsigned int buf_copy_front(void *from_buf, void *to_buf)
{
	int count = 0;
	char tmp[((Buffer_t *)from_buf)->t_size];
	if (from_buf && to_buf && ((Buffer_t *)from_buf)->t_size == ((Buffer_t *)to_buf)->t_size) {
		while (!buf_empty(from_buf)) {
			if (buf_full(to_buf))
				buf_get_back(&tmp[0], from_buf);
			buf_get_back(&tmp[0], from_buf);
			buf_push_front(&tmp[0], to_buf);
			++count;
		}
	}

	return count;
}
