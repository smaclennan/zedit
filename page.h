/* page.h - low level page defines
 * Copyright (C) 1988-2015 Sean MacLennan
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this project; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _page_h
#define _page_h

/* Generally, the bigger the page size the faster the editor however
 * the more wasted memory. A page size of 1k seems to be a very good trade off.
 * NOTE: DOS *requires* 1k pages for DOS_EMS.
 */
#define PSIZE		1024		/* size of page */
#define HALFP		(PSIZE / 2)	/* half the page size */

struct page {
#ifdef DOS_EMS
	Byte *pdata;			/* the page data */
	Byte emmpage;			/* 16k page */
	Byte emmoff;			/* offset in page */
#else
	Byte pdata[PSIZE];		/* the page data */
#endif
	int plen;			/* current length of the page */
	struct page *nextp, *prevp;	/* list of pages */
};

#define lastp(pg) ((pg)->nextp == NULL)

struct page *newpage(struct page *curpage);
bool bpagesplit(struct buff *buff);

#endif
