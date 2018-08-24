#include "buff.h"

/** True if mark1 follows mark2 */
bool mrkaftermrk(struct mark *mark1, struct mark *mark2)
{
	struct page *tpage;

	if (!mark1->mpage || !mark2->mpage || mark1->mbuff != mark2->mbuff)
		return false;        /* marks in different buffers */

	if (mark1->mpage == mark2->mpage)
		return mark1->moffset > mark2->moffset;
	if (mark1->mpage->nextp == mark2->mpage)
		return false;

	for (tpage = mark1->mpage->prevp;
		 tpage && tpage != mark2->mpage;
		 tpage = tpage->prevp)
		;
	return tpage != NULL;
}
