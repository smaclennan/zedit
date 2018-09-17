/* binstr.c - insert a string and format a string
 * Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this project; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdarg.h>
#include <ctype.h>
#include "buff.h"

/** @addtogroup buffer
 * @{
*/

/* \cond skip */
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
		bitoa(va_arg(ap, int), tmp);
		return out_str(out, tmp, saw_neg, len);
	case 'u':
		*fmt += n;
		butoa(va_arg(ap, unsigned), tmp);
		return out_str(out, tmp, saw_neg, len);
	case 'x':
		*fmt += n;
		bxtoa(va_arg(ap, unsigned), tmp);
		return out_str(out, tmp, saw_neg, len);
	case 'l':
		switch (*(*fmt + n + 1)) {
		case 'd':
			*fmt += n + 1;
			bitoa(va_arg(ap, long), tmp);
			return out_str(out, tmp, saw_neg, len);
		case 'u':
			*fmt += n + 1;
			butoa(va_arg(ap, unsigned long), tmp);
			return out_str(out, tmp, saw_neg, len);
		case 'x':
			*fmt += n + 1;
			bxtoa(va_arg(ap, unsigned long), tmp);
			return out_str(out, tmp, saw_neg, len);
		}
		/* fall through */
	default:
		return outchar(out, **fmt);
	}
}
/* \endcond */

/** Insert a string formatted with variable arguments into a buffer.
 *
 * Supports a subset of printf: %%s, %%d, %%u. Format can contain a width
 * and a minus (-) for left justify.
 *
 * @param buff The buffer to insert into.
 * @param fmt The format string.
 * @param ... The zero or more variable arguments.
 * @return 1 for success, 0 if output truncated.
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

/** Lower level interface to strfmt() when you already have the va_list.
 * @param str The output string.
 * @param len The length of the output string.
 * @param fmt The output format.
 * @param ap The va_list.
 * @return The number of bytes inserted.
 */
int strfmt_ap(char *str, int len, const char *fmt, va_list ap)
{
	struct outbuff out;
	int rc = 1;

	if (len < 1)
		return 0;

	out.buff = NULL;
	out.str = str;
	out.len = len;

	while (*fmt && rc) {
		if (*fmt == '%')
			rc = handle_format(&out, &fmt, ap);
		else
			rc = outchar(&out, *fmt);
		++fmt;
	}

	/* We leave room for the NULL */
	*out.str = 0;

	return len - out.len;
}

/** Poor man's snprintf.
 *
 * Supports a subset of printf: %%s, %%d, %%u. Format can contain a width
 * and a minus (-) for left justify.
 *
 * @param str The output string.
 * @param len The length of the output string.
 * @param fmt The output format.
 * @param ... The zero or more arguments.
 * @return The number of bytes inserted.
 */
int strfmt(char *str, int len, const char *fmt, ...)
{
	if (len < 1)
		return 0;

	va_list ap;
	va_start(ap, fmt);
	int n = strfmt_ap(str, len, fmt, ap);
	va_end(ap);

	return n;
}
/* @} */
