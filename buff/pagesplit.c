/* pagesplit.c - split the current page into two pages
 * Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this project; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "buff.h"

/** @addtogroup buffer
 * @{
*/

/** Low-level page function to split the current full page and return
 * a new page. Leaves dist in curpage. Point is moved to the new page
 * if required.
 * @param buff The buffer the Point is in.
 * @param dist The amount to leave in the old page (usually
 * HALFP). Must be <= PGSIZE.
 * @return The new page or NULL.
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
/* @} */
