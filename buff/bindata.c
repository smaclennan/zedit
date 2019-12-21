/* bindata.c - bulk data insert/append
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

/** Helper function to append data to a page, creating new pages as
 * needed. You must guarantee we are at the end of a page.
 * @param buff The buffer to append to.
 * @param data The data to append.
 * @param size The size of the data.
 * @return The number of bytes actually appended.
 */
static int bappendpage(struct buff *buff, const Byte *data, int size)
{
	int appended = 0;

	/* Fill the current page */
	int n, left = PGSIZE - curplen(buff);

	if (left > 0) {
		n = MIN(left, size);
		memcpy(buff->curcptr, data, n);
		buff->curcptr += n;
		buff->curchar += n;
		curplen(buff) += n;
		size -= n;
		data += n;
		appended += n;
		undo_add(buff, n);
	}

	/* Put the rest in new pages */
	while (size > 0) {
		struct page *npage = newpage(buff->curpage);

		if (!npage)
			return appended;
		makecur(buff, npage, 0);

		n = MIN(PGSIZE, size);
		memcpy(buff->curcptr, data, n);
		curplen(buff) = n;
		makeoffset(buff, n);
		undo_add(buff, n);
		size -= n;
		data += n;
		appended += n;
	}

	return appended;
}

/** Append data to the end of the buffer. Point is left at the end of
 * the buffer.
 * @param buff The buffer to append to.
 * @param data The data to append.
 * @param size The size of the data.
 * @return The amount of data actually appended.
 */
int bappend(struct buff *buff, const Byte *data, int size)
{
	btoend(buff);
	return bappendpage(buff, data, size);
}

/** Insert data at the current point. Point is left at the end of the
 * inserted data.
 * Simple version to start. Can use size / PGSIZE + 1 + 1 pages.
 * @param buff The buffer to insert into.
 * @param data The data to insert.
 * @param size The size of the data.
 * @return The amount of data actually inserted.
 */
int bindata(struct buff *buff, Byte *data, unsigned int size)
{
	struct page *npage;
	unsigned int n, copied = 0;

	/* If we can append... use append */
	if (buff->curchar == curplen(buff))
		return bappendpage(buff, data, size);

	n = PGSIZE - curplen(buff);
	if (n >= size) { /* fits in this page */
		struct mark *m;

		n = curplen(buff) - buff->curchar;
		memmove(buff->curcptr + size, buff->curcptr, n);
		memcpy(buff->curcptr, data, size);
		foreach_global_pagemark(m, buff->curpage)
			if (m->moffset >= buff->curchar)
				m->moffset += size;
		foreach_pagemark(buff, m, buff->curpage)
			if (m->moffset >= buff->curchar)
				m->moffset += size;
		buff->curcptr += size;
		buff->curchar += size;
		curplen(buff) += size;
		undo_add(buff, size);
		return size;
	}

	n = curplen(buff) - buff->curchar;
	if (n > 0) {
		if (!pagesplit(buff, buff->curchar))
			return copied;

		/* Copy as much as possible to the end of this page */
		n = MIN(PGSIZE - buff->curchar, size);
		memcpy(buff->curcptr, data, n);
		data += n;
		size -= n;
		undo_add(buff, n);
		copied += n;
		buff->curcptr += n;
		buff->curchar += n;
		curplen(buff) = buff->curchar;
	}

	while (size > 0) {
		npage = newpage(buff->curpage);
		if (!npage)
			break;

		n = MIN(PGSIZE, size);
		memcpy(npage->pdata, data, n);
		data += n;
		size -= n;
		undo_add(buff, n);
		copied += n;

		makecur(buff, npage, n);
		curplen(buff) = n;
	}

	return copied;
}
/* @} */
