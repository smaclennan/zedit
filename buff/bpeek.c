/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include "buff.h"

/** @addtogroup buffer
 * @{
 */

/** Peek the byte before the point. Does not move the point. Returns
 * LF at start of buffer. Much more efficient than moving the Point.
 * @param buff The buffer to peek.
 * @return The byte before the point. Returns LF at start of buffer.
 */
Byte bpeek(struct buff *buff)
{
	if (buff->curchar > 0)
		return *(buff->curcptr - 1);
	if (buff->curpage->prevp) {
		struct page *prev = buff->curpage->prevp;
		return prev->pdata[prev->plen - 1];
	}
	/* We are at the start of the buffer.
	 * Pretend we are at the start of a line.
	 * Needed for delete-to-eol and step in reg.c.
	 */
	return '\n';
}
/* @} */
