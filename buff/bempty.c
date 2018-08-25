#include "buff.h"

/** Delete all bytes from a buffer and leave it with one empty page
 * (ala bcreate()). More efficient than bdelete(blength(buff)) since it
 * works on pages rather than bytes.
 */
void bempty(struct buff *buff)
{
	struct mark *btmark;

#if HUGE_FILES
	bhugecleanup(buff);
#endif

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
