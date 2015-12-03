#include "string.h"

int strncmp(const char *cs, const char *ct, int count)
{
	unsigned char c1, c2;
	while (count--) {
		c1 = *cs++;
		c2 = *ct++;
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
		if (!c1)
			break;
	}
	return 0;
}

char *strpbrk(const char *cs, const char *ct)
{
	const char *sc1, *sc2;

	for (sc1 = cs; *sc1 != '\0'; ++sc1) {
		for (sc2 = ct; *sc2 != '\0'; ++sc2) {
			if (*sc1 == *sc2)
				return (char *)sc1;
		}
	}
	return NULL;
}

char *strsep(char **s, const char *ct)
{
	char *sbegin = *s;
	char *end;

	if (sbegin == NULL)
		return NULL;

	end = strpbrk(sbegin, ct);
	if (end)
		*end++ = '\0';

	*s = end;
	return sbegin;
}

int strlen(const char *s)
{
	const char *sc;

	for (sc = s; *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}

char *strcpy(char *dest, const char *src)
{
	char *tmp = dest;

	while ((*dest++ = *src++) != '\0')
		/* nothing */;
	return tmp;
}

char *strcat(char *dest, const char *src)
{
	char *tmp = dest;

	while (*dest)
		dest++;
	while ((*dest++ = *src++) != '\0')
		;
	return tmp;
}

static inline char isspace(char c)
{
	return (c == ' ') ? 1 : 0;
}

int strtoi(const char *str)
{
	char sgn = 0;
	unsigned long result = 0;
	unsigned char c;
	while (isspace(*str))
		++str;

	if (*str == '-') {
		sgn = 1;
		++str;
	} else if (*str == '+')
		++str;

	while (*str) {
		c = *str;
		c = c <= '9' ? c - '0' : 0xff;
		if (c > 10)
			break;

		result = result * 10 + c;
		/* FIXME: overflow */
		++str;
	}

	return sgn ? ((int) result) * -1 : ((int) result);
}

static inline void reverse(char *src, char *dest)
{
	char tmp;
	while (dest > src) {
		tmp = *dest;
		*dest = *src;
		*src = tmp;
		--dest;
		++src;
	}
}

void itostr(int x, char *str)
{
	char *ptr;
	if (x == 0) {
		*str = '0';
		*(str + 1) = '\0';
		return;
	}

	if (x < 0) {
		*str = '-';
		x *= -1;
		++str;
	}

	ptr = str;
	while (x > 0) {
		*str = x % 10 + '0';
		x /= 10;
		++str;
	}

	*str = '\0';
	--str;
	reverse(ptr, str);
}

