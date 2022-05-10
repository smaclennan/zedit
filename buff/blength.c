/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include "buff.h"

/** @addtogroup buffer
 * @{
 */

/** Return the length of the buffer.
 * @param buff The buffer to return the length of.
 * @return The length of the buffer in bytes.
 */
unsigned long blength(struct buff *buff)
{
	struct page *tpage;
	unsigned long len = 0;

	for (tpage = buff->firstp; tpage; tpage = tpage->nextp)
		len += tpage->plen;

	return len;
}
/* @} */
