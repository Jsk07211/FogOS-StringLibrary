#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "user/string.h"

#include <stdarg.h>

static char digits[] = "0123456789";

int 
scanf(const char *restrict format, ...)
{
    char *buf = 0;
    uint sz = 0;
    int bytes_read = getline(&buf, &sz, 0);
    char *copy = buf;

	if (bytes_read < 0)
			return -1;
			
	// Conventional name for va_list object
	va_list ap;
	va_start(ap, format);

	char *s;
	int c, i, state, success;
	state = 0;
	success = 0;				// Increments for each successful write into buffer

	for (i = 0; format[i]; i++) {
		// Return if user string once does not match up with format string
		if (!*buf)
			break;
	
		c = format[i] & 0xff;	// Leaves only the last 8 bits of the original
		if (state == 0) {
			if (c == '%') {
				state = '%';
			} else {
				buf++;			// iterate through string since no match
			}
		} else if (state == '%') {
			if (c == 'd') {
				int *d = va_arg(ap, int *);

				int offset;
				int is_neg = 0;	// by default, set to non-negative
				
				if (*buf == '-') {
					is_neg = 1;
					buf++;	
				}

				offset = strspn(buf, digits);

				// Convert str to int if input is valid sequence of digits (not too short, not too long)
				if (offset > 0 && offset < 11) {
					char temp[offset + 1];
					memset(temp, 0, offset + 1);
					strncpy(temp, buf, offset);
					int val = atoi(temp);
					
					if (is_neg)
						val *= -1;
					
					*d = val;
					buf += offset;
					success++;	
				}
			} 
			else if (c == 's') {
				s = va_arg(ap, char *);

				// No buffer space allocated, not successfully copied
				if (s == 0) {
					// early return since error is hit
					va_end(ap);
					return success;	
				} else {
					// scanf stops reading when whitespace
					int offset = strcspn(buf, " \t\n\r");
					// assumes user allocates sufficient space, similar to C Library scanf
					strncpy(s, buf, offset - 1);
					buf += offset - 1;
					success++;	
				}
			}
			state = 0;
		}
	}
	free(copy);		// free malloc from getline
	va_end(ap);
	return success;
}
