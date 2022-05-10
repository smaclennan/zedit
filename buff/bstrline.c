/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include "buff.h"

/** @addtogroup buffer
 * @{
 */

/** Store the current Point line. Leaves Point at the start of the
 * next line.
 * @param buff The buffer the Point is in.
 * @param[out] str The string to store the line to.
 * @param len The length of the string.
 * @return The actual length of the line. Can be > string length.
 */
int bstrline(struct buff *buff, char *str, int len)
{
	int count = 0;

	if (len == 0)
		return 0;

	tobegline(buff);

	while (len > 1 && !bisend(buff) && *buff->curcptr != '\n') {
		*str++ = *buff->curcptr;
		--len;
		++count;
		bmove1(buff);
	}
	*str = 0;

	while (!bisend(buff) && *buff->curcptr != '\n') {
		++count;
		bmove1(buff);
	}
	if (*buff->curcptr == '\n')
		bmove1(buff);

	return count;
}
/* @} */
