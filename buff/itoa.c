#include "buff.h"

/** Helper function for itoa() and utoa(). */
static void reverse_string(char *start, char *end)
{
	int len = (end - start) / 2;
	--end;
	for (int i = 0; i < len; ++i) {
		char temp = *end;
		*end = *start;
		*start = temp;
		start++;
		end--;
	}
}

char *itoa(int val, char *out)
{
	char *p = out;
	int neg = val < 0;

	if (val == 0) {
		strcpy(out, "0");
		return out;
	}

	if (neg)
		val = -val;

	while (val > 0) {
		*p++ = (val % 10) + '0';
		val /= 10;
	}
	if (neg) *p++ = '-';
	*p = 0;

	reverse_string(out, p);
	return out;
}

/* Returns the end of the output */
char *_utoa(unsigned val, char *out)
{
	char *p = out;

	if (val == 0) {
		strcpy(out, "0");
		return out + 1;
	}

	while (val > 0) {
		*p++ = (val % 10) + '0';
		val /= 10;
	}
	*p = 0;

	reverse_string(out, p);
	return p;
}

char *utoa(unsigned val, char *out)
{
	_utoa(val, out);
	return out;
}
