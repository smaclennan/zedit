/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include "buff.h"

/** @addtogroup buffer
 * @{
 */

/** Delete all bytes from a buffer and leave it with one empty page
 * (ala bcreate()). More efficient than bdelete(blength(buff)) since it
 * works on pages rather than bytes.
 * @param buff The buffer to empty.
 */
void bempty(struct buff *buff)
{
	struct mark *btmark;

	bhugecleanup(buff);

	makecur(buff, buff->firstp, 0);
	curplen(buff) = 0;
	while (buff->curpage->nextp)
		freepage(buff, buff->curpage->nextp);

	foreach_global_buffmark(buff, btmark)
		if (btmark->mpage) {
			btmark->mpage = buff->firstp;
			btmark->moffset = 0;
		}
	foreach_buffmark(buff, btmark)
		if (btmark->mpage) {
			btmark->mpage = buff->firstp;
			btmark->moffset = 0;
		}

	undo_clear(buff);
	bsetmod(buff);
}
/* @} */
