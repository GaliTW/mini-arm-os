#ifndef __BUFFER_H_
#define __BUFFER_H_

void *buf_init(char *raw_buf, unsigned int type_size, unsigned int total_size);
void buf_release(void *buf);
void buf_clear(void *buf);
unsigned int buf_size(void *buf);
unsigned int buf_total_size(void *buf);
unsigned char buf_empty(void *buf);
unsigned char buf_full(void *buf);
unsigned char buf_get_front(char *e, void *buf);
unsigned char buf_get_back(char *e, void *buf);
unsigned char buf_push_front(char *e, void *buf);
unsigned char buf_push_back(char *e, void *buf);
char *buf_seek_front(void *buf);
char *buf_seek_back(void *buf);
unsigned int buf_copy_front(void *from_buf, void *to_buf);

#endif
