/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"

/* This file contains the memory based version of the low-level
 * buffer routines.
 */

static int NumPages = 0;

/* Initialize the buffer stuff */
void Binit()
{
	initScrnmarks();			/* init the screen marks and mark list */
}


/* Create a new memory page and link into chain */
Page *Newpage(tbuff, ppage, npage)
Buffer *tbuff;
Page *ppage, *npage;
{
	Page *new;

	if((new = (Page *)malloc(sizeof(Page))) != NULL)
	{
		new->nextp = npage;
		new->prevp = ppage;
		npage ? (npage->prevp = new) : (tbuff->lastp = new);
		ppage ? (ppage->nextp = new) : (tbuff->firstp = new);
		new->plen  = 0;
		new->lines = EOF;		/* undefined */
		++NumPages;
	}
	return new;
}


/* Free a memory page */
void Freepage(tbuff, page)
Buffer *tbuff;
Page *page;
{
	if(page->nextp)
		page->nextp->prevp = page->prevp;
	else
		tbuff->lastp = page->prevp;
	if(page->prevp)
		page->prevp->nextp = page->nextp;
	else
		tbuff->firstp = page->nextp;
	free((char *)page);
	--NumPages;
}


/* Make page current*/
void Makecur(page)
Page *page;
{
	extern Boolean Curmodf;

	if(Curpage == page) return;
	if(Curpage)
	{
		Curpage->plen = Curplen;
		if(Curmodf || Curpage->lines == EOF)
			Curpage->lines = Cntlines(Curplen);
	}
	Curpage = page;
	Cpstart = page->pdata;
	Curmodf = FALSE;
	Curplen = Curpage->plen;
}


/* Split the current (full) page. */
Boolean Pagesplit()
{
	extern Boolean Curmodf;
	Page *new;
	Mark *btmark;

	if((new = Newpage(Curbuff, Curpage, Curpage->nextp)) == NULL)
		return FALSE;

	memmove(new->pdata, Cpstart + (PSIZE / 2), PSIZE / 2);
	Curmodf = TRUE;
	Curplen = PSIZE / 2;
	new->plen = PSIZE / 2;
	for(btmark = Mrklist; btmark; btmark = btmark->prev)
		if(btmark->mpage == Curpage && btmark->moffset >= (PSIZE / 2))
		{
			btmark->mpage = new;
			btmark->moffset -= PSIZE / 2;
		}
	if(Curchar >= (PSIZE / 2))
	{	/* new page has Point in it */
		Makecur(new);
		Makeoffset(Curchar - (PSIZE / 2));
	}
	return TRUE;
}


Proc Zstat()
{
	extern int Numbuffs;

	sprintf(PawStr, "Buffers: %d   Pages: %d", Numbuffs, NumPages);
#if XWINDOWS
	AddWindowSizes(PawStr + strlen(PawStr));
#endif
	Echo(PawStr);
}


/* High level write for non-block writes.
 * Buffers up the input until a PSIZE block is reached,
 * then sends it to XBwrite.
 * Can only be used on ONE file at a time!
 * A 'XBput( fd, NULL, EOF )' call should be made before closing the file.
 * NOTE: Special code for Zedit added for AddNL
 */
#if SLOW_DISK
Boolean XBput(fd, addr, len)
int fd;
Byte *addr;
unsigned len;
{
	static Byte buf[PSIZE];
	static int buflen = 0, lastch = 0;
	int wrlen, rc = TRUE;

	if(len == 0) return TRUE;
	if(len == EOF)
	{	/* flush the buffer and reset */
		if(Vars[VADDNL].val)
		{
			if(buflen > 0) lastch = buf[buflen - 1];
			if(lastch != '\n') XBput(fd, (Byte *)"\n", 1);
		}
		rc = write(fd, buf, buflen) == buflen;
		buflen = lastch = 0;
	}
	else
	{
		wrlen = (buflen + len > PSIZE) ? (PSIZE - buflen) : len;
		memcpy(&buf[buflen], addr, wrlen);
		buflen += wrlen;
		if(buflen == PSIZE)
		{
			rc = write(fd, buf, PSIZE) == PSIZE;
			lastch = buf[PSIZE - 1];
			buflen = 0;
			rc &= XBput(fd, &addr[wrlen], len - wrlen);
		}
	}
	return rc;
}
#else
Boolean XBput(fd, addr, len)
int fd;
Byte *addr;
unsigned len;
{
	static int lastch = 0;

	if(len == 0) return TRUE;
	if(len == EOF)
	{	/* handle ADDNL */
		if(Vars[VADDNL].val && lastch != '\n')
		{
			char buf = '\n';
			return write(fd, &buf, 1) == 1;
		}
		else
		{
			lastch = 0;
			return TRUE;
		}
	}

	lastch = addr[len - 1];
	return write(fd, addr, len) == len;
}
#endif
