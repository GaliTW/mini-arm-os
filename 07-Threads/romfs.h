#ifndef __G_ROMFS_H_
#define __G_ROMFS_H_

extern unsigned char _sromfs;
extern unsigned char _eromfs;

unsigned char fs_init(unsigned char *addr);
void normal_path(const char *path, char *normal_path);
void *openfile(const char* file);
void *opendir(const char* directory);
void printfile(void *filentry);
void printdir(void *direntry);
void *findentry(const char *path, uint8_t type);

#endif

