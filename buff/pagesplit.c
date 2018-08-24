#include "buff.h"

/** Split the current full page and return a new page. Leaves dist in
 * curpage. Point is moved to the new page if required.
 */
struct page *pagesplit(struct buff *buff, unsigned dist)
{
	struct page *curpage, *newp;
	struct mark *btmark;
	int newsize;

	if (dist > PGSIZE)
		return NULL;

	curpage = buff->curpage;
	newp = newpage(curpage);
	if (!newp)
		return NULL;

	newsize = curpage->plen - dist;
	memcpy(newp->pdata, curpage->pdata + dist, newsize);
	curpage->plen = dist;
	newp->plen = newsize;

	foreach_global_pagemark(btmark, curpage)
		if (btmark->moffset >= dist) {
			btmark->mpage = newp;
			btmark->moffset -= dist;
		}
	foreach_pagemark(buff, btmark, curpage)
		if (btmark->moffset >= dist) {
			btmark->mpage = newp;
			btmark->moffset -= dist;
		}

	if (buff->curchar > dist)
		/* new page has Point in it */
		makecur(buff, newp, buff->curchar - dist);

	return newp;
}
