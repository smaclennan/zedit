#include <stdarg.h>
#include <ctype.h>
#include "buff.h"

/** Helper function for binstr(). */
static char valid_format(const char *fmt, int *saw_neg, int *len, int *n)
{
	*saw_neg = *len = *n = 0;
	++fmt; ++*n; /* skip % */
	if (*fmt == '-') {
		*saw_neg = 1;
		++fmt; ++*n;
	}
	while (isdigit(*fmt)) {
		*len = *len * 10 + *fmt - '0';
		++fmt; ++*n;
	}
	if (*fmt == 'u' || *fmt == 'd')
		*saw_neg = 0;
	return *fmt;
}

/** Helper function for binstr(). */
static int out_str(struct buff *buff, const char *s, int saw_neg, int len)
{
	int slen = strlen(s);

	if (saw_neg == 0)
		while (slen++ < len)
			if (!binsert(buff, ' '))
				return 0;
	while (*s)
		if (!binsert(buff, *s++))
			return 0;
	while (slen++ < len)
		if (!binsert(buff, ' '))
			return 0;
	return 1;
}

/** Helper function for binstr(). */
static int handle_format(struct buff *buff, const char **fmt, va_list ap)
{
	char tmp[12];
	int saw_neg, len, n;

	switch (valid_format(*fmt, &saw_neg, &len, &n)) {
	case 's':
		*fmt += n;
		return out_str(buff, va_arg(ap, char *), saw_neg, len);
	case 'd':
		*fmt += n;
		itoa(va_arg(ap, int), tmp);
		return out_str(buff, tmp, saw_neg, len);
	case 'u':
		*fmt += n;
		utoa(va_arg(ap, unsigned), tmp);
		return out_str(buff, tmp, saw_neg, len);
	default:
		return binsert(buff, **fmt);
	}
}

/** Insert a string. Uses variable arguments.
 *
 * Supports a subset of printf: %%s, %%d, %%u. Format can contain a width
 * and a minus (-) for left justify.
 */
int binstr(struct buff *buff, const char *fmt, ...)
{
	va_list ap;
	int rc = 1;

	va_start(ap, fmt);
	while (*fmt && rc) {
		if (*fmt == '%')
			rc = handle_format(buff, &fmt, ap);
		else
			rc = binsert(buff, *fmt);
		++fmt;
	}
	va_end(ap);

	return rc;
}
