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

#define BUFFERED
#include "strfmt.c"

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
	struct outbuff out = { .buff = buff, .str = NULL, .n = 0 };
	va_list ap;

	va_start(ap, fmt);
	int rc = __strfmt(&out, fmt, ap);
	va_end(ap);

	return rc;
}
/* @} */
