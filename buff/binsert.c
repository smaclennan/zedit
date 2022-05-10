/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include "buff.h"

/** @addtogroup buffer
 * @{
 */

/** Insert a byte into a buffer. Grow the buffer if needed.
 * @param buff The buffer to insert into.
 * @param byte The byte to insert.
 * @return 1 on success, 0 if we cannot allocate space for the byte.
 */
int binsert(struct buff *buff, Byte byte)
{
	struct mark *btmark;

	if (curplen(buff) == PGSIZE && !pagesplit(buff, HALFP))
		return 0;
	memmove(buff->curcptr + 1,
		buff->curcptr,
		curplen(buff) - buff->curchar);
	*buff->curcptr++ = byte;
	buff->curchar++;
	curplen(buff)++;
	buff->bmodf = 1;

	undo_add(buff, 1);

	foreach_global_pagemark(btmark, buff->curpage)
		if (btmark->moffset >= buff->curchar)
			++(btmark->moffset);
	foreach_pagemark(buff, btmark, buff->curpage)
		if (btmark->moffset >= buff->curchar)
			++(btmark->moffset);

	bsetmod(buff);
	return 1;
}
/* @} */
