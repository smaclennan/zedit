#include <stdarg.h>
#include <ctype.h>
#include "buff.h"

struct outbuff {
	struct buff *buff;
	char *str;
	int len;
};

/** Helper function for binstr(). */
static int outchar(struct outbuff *out, char ch)
{
	if (out->buff)
		return binsert(out->buff, ch);
	if (out->len > 1) {
		*out->str++ = ch;
		--out->len;
		return 1;
	}
	return 0;
}

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
static int out_str(struct outbuff *out, const char *s, int saw_neg, int len)
{
	int slen = strlen(s);

	if (saw_neg == 0)
		while (slen++ < len)
			if (!outchar(out, ' '))
				return 0;
	while (*s)
		if (!outchar(out, *s++))
			return 0;
	while (slen++ < len)
		if (!outchar(out, ' '))
			return 0;
	return 1;
}

/** Helper function for binstr(). */
static int handle_format(struct outbuff *out, const char **fmt, va_list ap)
{
	char tmp[12];
	int saw_neg, len, n;

	switch (valid_format(*fmt, &saw_neg, &len, &n)) {
	case 's':
		*fmt += n;
		return out_str(out, va_arg(ap, char *), saw_neg, len);
	case 'd':
		*fmt += n;
		itoa(va_arg(ap, int), tmp);
		return out_str(out, tmp, saw_neg, len);
	case 'u':
		*fmt += n;
		utoa(va_arg(ap, unsigned), tmp);
		return out_str(out, tmp, saw_neg, len);
	default:
		return outchar(out, **fmt);
	}
}

/** Insert a string. Uses variable arguments.
 *
 * Supports a subset of printf: %%s, %%d, %%u. Format can contain a width
 * and a minus (-) for left justify.
 *
 * Returns 1 if success, 0 if output truncated.
 */
int binstr(struct buff *buff, const char *fmt, ...)
{
	struct outbuff out;
	va_list ap;
	int rc = 1;

	out.buff = buff;

	va_start(ap, fmt);
	while (*fmt && rc) {
		if (*fmt == '%')
			rc = handle_format(&out, &fmt, ap);
		else
			rc = binsert(buff, *fmt);
		++fmt;
	}
	va_end(ap);

	return rc;
}

/** Poor man's snprintf.
 *
 * Supports a subset of printf: %%s, %%d, %%u. Format can contain a width
 * and a minus (-) for left justify.
 *
 * Returns bytes inserted.
 */
int strfmt(char *str, int len, const char *fmt, ...)
{
	struct outbuff out;
	va_list ap;
	int rc = 1;

	if (len < 1)
		return 0;

	out.buff = NULL;
	out.str = str;
	out.len = len;

	va_start(ap, fmt);
	while (*fmt && rc) {
		if (*fmt == '%')
			rc = handle_format(&out, &fmt, ap);
		else
			rc = outchar(&out, *fmt);
		++fmt;
	}
	va_end(ap);

	/* We leave room for the NULL */
	*out.str = 0;

	return len - out.len;
}
