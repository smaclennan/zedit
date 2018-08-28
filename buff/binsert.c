#include "buff.h"

/** Insert a byte into a buffer. Returns 0 if more space is needed
 * but we cannot allocate it.
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
