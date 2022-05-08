/* create a buffer
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

/** Create a buffer and allocate the first page.
 * @return The newly created buffer or NULL.
 */
struct buff *bcreate(void)
{
	struct buff *buf = (struct buff *)calloc(1, sizeof(struct buff));

	if (buf) {
		buf->firstp = newpage(buf, NULL);
		if (!buf->firstp) {
			/* bad news, de-allocate */
			free(buf);
			return NULL;
		}
		makecur(buf, buf->firstp, 0);
	}
	return buf;
}

/** Low-level page function to create a new memory page and link into
 * chain after curpage.
 * @param buf The buffer to add the page to.
 * @param curpage The current page. Can be NULL.
 * @return The new page or NULL.
 */
struct page *newpage(struct buff *buff, struct page *curpage)
{
	struct page *page = (struct page *)calloc(1, sizeof(struct page));

	if (page) {
		if (curpage) {
			page->prevp = curpage;
			if (curpage->nextp) {
				page->nextp = curpage->nextp;
				curpage->nextp->prevp = page;
			} else
				buff->lastp = page;
			curpage->nextp = page;
		} else
			buff->lastp = page;
	}

	return page;
}
/* @} */
