#ifndef __GSTRING_H_
#define __GSTRING_H_

#ifndef NULL
#define NULL '\0'
#endif

int strncmp(const char *cs, const char *ct, int count);
char *strsep(char **s, const char *ct);
int strlen(const char *s);
char *strcpy(char *dest, const char *src);
char *strcat(char *dest, const char *src);
int strtoi(const char *str);
void itostr(int x, char *str);

#endif
