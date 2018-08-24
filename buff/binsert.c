#include "buff.h"

/** Insert a byte into a buffer. Returns false if more space is needed
 * but we cannot allocate it.
 */
bool binsert(struct buff *buff, Byte byte)
{
	struct mark *btmark;

	if (curplen(buff) == PGSIZE && !pagesplit(buff, HALFP))
		return false;
	memmove(buff->curcptr + 1,
		buff->curcptr,
		curplen(buff) - buff->curchar);
	*buff->curcptr++ = byte;
	buff->curchar++;
	curplen(buff)++;
	buff->bmodf = true;

	undo_add(buff, 1);

	foreach_global_pagemark(btmark, buff->curpage)
		if (btmark->moffset >= buff->curchar)
			++(btmark->moffset);
	foreach_pagemark(buff, btmark, buff->curpage)
		if (btmark->moffset >= buff->curchar)
			++(btmark->moffset);

	bsetmod(buff);
	return true;
}
