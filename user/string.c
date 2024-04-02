#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "user/string.h"

char
*strncpy(char *dst, const char *src, uint n)
{
	char *walk = dst;
	while ((*walk++ = *src++) && n > 0)
		n--;

	*walk = '\0'; 

	return dst;	
}


int
strncmp(const char *s1, const char *s2, uint n)
{
	while (*s1 && *s1 == *s2 && n > 1) 
		s1++, s2++, n--;

	// return compares last 2 chars safely
	return (uchar)*s1 - (uchar)*s2;
}

char 
*strncat(char *restrict s1, const char *restrict s2, uint n)
{
	// copies without overriding chars already in s1
	strncpy(s1 + strlen(s1), s2, n);
	return s1;
}

char
*strstr(const char *haystack, const char *needle)
{
	if (!*needle) {
		return (char *)haystack;
	}	

	int substr_len = strlen(needle) - 1;

	// const char* shows that func will not change char str
	char *walk = (char *)haystack;
	while (*walk) {
		if (*walk == *needle) {
			if (!strncmp(walk, needle, substr_len)) {
				return walk;
			}
		}
		walk++;
	}
	return 0;
}

uint 
strspn(const char *s, const char *accept)
{
	if (s == 0)
		return 0;

	int accept_len = strlen(accept);
	char *walk = (char *)s;
	uint span_len = 0;

	while (*walk) {
		for (int i = 0; i < accept_len; i++) {
			if (*walk == accept[i]) {
				span_len++;
				break;
			}

			// If it hits this line, then *walk has no match with anything in charset
			if (i == accept_len - 1) {
				return span_len;
			}
		}
		walk++;	
	}
	return span_len;
}

uint 
strcspn(const char *s, const char *reject)
{
	if (s == 0)
		return 0;

	int reject_len = strlen(reject);
	char *walk = (char *)s;
	uint span_len = 0;

	while (*walk) {
		for (int i = 0; i < reject_len; i++) {
			if (*walk == reject[i]) {
				return span_len;
			}
		}
		// If it hits this line, then *walk has no match with anything in charset
		span_len++;
		walk++;	
	}
	return span_len;
}

char
*strtok_r(char *restrict str, const char *restrict sep, char **restrict last) {
	if (str == 0) { // str is null, check last instead
		str = *last;
	}
	
	if (*str == '\0') { // last is also null, point last to str
		*last = str;
		return 0;
	}

	str = str + strspn(str, sep); // jump to next occurance
	if (*str == '\0') { // point last to str
		*last = str;
		return 0;
	}

	char *end = str + strcspn(str, sep); // get to the end of token
	if (*end == '\0') { // if token is already null pointer, make last point to end of token
		*last = end;
	} else { // terminate the token with null terminator point last to 1 space beyond it
		*end = '\0';
		*last = end + 1;
	}
	return str;
}

char
*strtok(char *restrict str, const char *restrict sep) {
	static char *last;
	return strtok_r(str, sep, &last);
}

char
*strsep(char **stringp, const char *delim) {
	if (*stringp == 0) // string is null
		return 0;

	char *retval = *stringp;
	char *end = *stringp + strcspn(*stringp, delim); // get to the end of first occurance
	if (*end == '\0') {
		*stringp = 0;
	} else {
		*end = '\0';
		*stringp = end + 1;
	}
	
	return retval;
}
