/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include "buff.h"

/** @addtogroup buffer
 * @{
 */

/** Move the point to a given absolute offset in the buffer.
 * @param buff The buffer to move the Point in.
 * @param offset The offset to move to in bytes.
 */
void boffset(struct buff *buff, unsigned long offset)
{
	struct page *tpage;

	/* find the correct page */
	for (tpage = buff->firstp; tpage->nextp; tpage = tpage->nextp) {
		if (tpage->plen >= offset)
			break;
		offset -= tpage->plen;
	}

	makecur(buff, tpage, offset);
}
/* @} */
