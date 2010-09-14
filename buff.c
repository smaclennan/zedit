/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"
#include <setjmp.h>
#include <sys/stat.h>
#include <time.h>

Boolean		Curmodf;			/* page modified?? */
Byte		*Cpstart;			/* pim data start */
Byte 		*Curcptr;			/* current character */
int 		Curchar;			/* current offset in Cpstart */
int 		Curplen;			/* current page length */
Buffer		*Bufflist	= NULL;	/* the buffer list */
Buffer 		*Curbuff	= NULL;	/* the current buffer */
Mark		*Mrklist	= NULL;	/* the marks list */
Page		*Curpage	= NULL;	/* the current page */


/* Copy from Point to tmark to tbuff. Returns number of bytes copied. Caller must handle undo. */
int Bcopyrgn(Mark *tmark, Buffer *tbuff)
{
	Buffer *sbuff;
	Mark *ltmark, *btmark;
	Boolean flip;
	int  srclen, dstlen;
	Byte *spnt;
	int copied = 0;

	if(tbuff == Curbuff)
		return 0;

	flip = Bisaftermrk(tmark);
	if (flip)
		Bswappnt(tmark);

	ltmark = Bcremrk();
	sbuff = Curbuff;
	while(Bisbeforemrk(tmark)) {
		if( Curpage == tmark->mpage )
			srclen = tmark->moffset - Curchar;
		else
			srclen = Curplen - Curchar;
		Curmodf = TRUE;
		spnt = Curcptr;

		Bswitchto(tbuff);
		dstlen = PSIZE - Curplen;
		if (dstlen == 0) {
			if (Pagesplit())
				dstlen = PSIZE - Curplen;
			else {
				Bswitchto( sbuff );
				break;
			}
		}
		if( srclen < dstlen )
			dstlen = srclen;
		/* Make a gap */
		memmove( Curcptr + dstlen, Curcptr, Curplen - Curchar );
		/* and fill it in */
		memmove( Curcptr, spnt, dstlen );
		Curplen += dstlen;
		copied += dstlen;
		for( btmark = Mrklist; btmark; btmark = btmark->prev )
			if( btmark->mpage == Curpage && btmark->moffset > Curchar )
					btmark->moffset += dstlen;
		Makeoffset( Curchar + dstlen );
		Vsetmod( FALSE );
		Curmodf = TRUE;
		Curbuff->bmodf = MODIFIED;
		Bswitchto( sbuff );
		Bmove( dstlen );
	}

	Bpnttomrk( ltmark );
	Unmark( ltmark );

	if (flip)
		Bswappnt(tmark);

	return copied;
}


/* Create a buffer.   Returns a pointer to the buffer descriptor. */
Buffer *Bcreate()
{
	Buffer *new;
	Page *fpage;

	if((new = (Buffer *)malloc(sizeof(Buffer))) != NULL)
	{
		/* initialize before Newpage call! */
		memset(new, 0, sizeof(Buffer));
		if((fpage = Newpage(new, NULL, NULL)) == NULL)
		{ /* bad news, de-allocate */
			free((char *)new);
			return NULL;
		}
		new->mark = Bcremrk();
		new->mark->mbuff = new;
		new->pnt_page = new->mark->mpage = fpage;
		new->bmode = 	(Vars[ VNORMAL ].val ? NORMAL    : TEXT) |
						(Vars[ VEXACT ].val  ? EXACT     : 0) |
						(Vars[ VOVWRT  ].val ? OVERWRITE : 0);
#if PIPESH || XWINDOWS
		new->child = EOF;
#endif
	}
	return new;
}


/* Create a mark at the current point and add it to the list.
 * If we are unable to alloc, longjmp.
 */
Mark *Bcremrk()
{
	extern jmp_buf zenv;
	Mark *new;

	if((new = (Mark *)malloc(sizeof(Mark))) == NULL)
		longjmp(zenv, -1);	/* ABORT */
	Bmrktopnt(new);
	new->prev = Mrklist;		/* add to end of list */
	new->next = NULL;
	if(Mrklist) Mrklist->next = new;
	Mrklist = new;
	return new;
}


#ifdef __STDC__
Boolean Bcrsearch(register Byte what)
#else
Boolean Bcrsearch(what)
register Byte what;
#endif
{
	while(1)
	{
		if(Curchar <= 0)
			if(Curpage == Curbuff->firstp)
				return FALSE;
			else
			{
				Makecur(Curpage->prevp);
				Makeoffset(Curplen - 1);
			}
		else
			Makeoffset(Curchar - 1);
		if(Buff() == what)
			return TRUE;
	}
}


Boolean Bcsearch(Byte what)
{
	Byte *n;

	if (Bisend())
		return FALSE;

	while ((n = (Byte *)memchr(Curcptr, what, Curplen - Curchar)) == NULL)
		if(Curpage == Curbuff->lastp) {
			Makeoffset(Curplen);
			return FALSE;
		} else {
			Makecur(Curpage->nextp);
			Makeoffset(0);
		}

	Makeoffset(n - Cpstart);
	Bmove1();
	return TRUE;
}


/* Delete the buffer and its pages. */
Boolean Bdelbuff(Buffer *tbuff)
{
	if (!tbuff)
		return TRUE;

	if( tbuff == Curbuff ) { /* switch to a safe buffer */
		if (tbuff->next)
			Bswitchto(tbuff->next);
		else if (tbuff->prev)
			Bswitchto(tbuff->prev);
		else {
			Error( "Last Buffer." );
			return( FALSE );
		}
	}

#if PIPESH || XWINDOWS
	if( tbuff->child != EOF )
		Unvoke( tbuff, TRUE );
#endif

	while (tbuff->firstp)					/* delete the pages */
		Freepage( tbuff, tbuff->firstp );
	Unmark( tbuff->mark );					/* free the user mark */
	if( tbuff == Bufflist )					/* unlink from the list */
		Bufflist = tbuff->next;
	if( tbuff->prev )
		tbuff->prev->next = tbuff->next;
	if( tbuff->next )
		tbuff->next->prev = tbuff->prev;
	free( (char *)tbuff );					/* free the buffer proper */

	undo_clear(tbuff);

	return TRUE;
}

/* Delete quantity characters. */
void Bdelete(unsigned quantity)
{
	int quan, noffset;
	Page *tpage;
	register Mark *tmark;

	while(quantity) {
		/* Delete as many characters as possible from this page */
		if(Curchar + quantity > Curplen)
			quan = Curplen - Curchar;
		else
			quan = quantity;
		if(quan < 0)
			quan = 0; /* May need to switch pages */
		Curplen -= quan;

		undo_del(quan);

		memmove(Curcptr, Curcptr + quan, Curplen - Curchar);
		if( Curpage == Curbuff->lastp )
			quantity = 0;
		else
			quantity -= quan;
		Curbuff->bmodf = MODIFIED;
		Curmodf = TRUE;
		if(Curplen == 0 && (Curpage->nextp || Curpage->prevp))
		{	/* We deleted entire page. */
			tpage = Curpage->nextp;
			noffset = 0;
			if(tpage == NULL)
			{
				tpage = Curpage->prevp;
				noffset = tpage->plen;
			}
			for(tmark = Mrklist; tmark; tmark = tmark->prev)
				if( tmark->mpage == Curpage )
				{
					tmark->mpage = tpage;
					tmark->moffset = noffset;
				}
			Freepage( Curbuff, Curpage );
			Curpage = NULL;
		}
		else
		{
			tpage = Curpage;
			noffset = Curchar;
			if( (noffset >= Curplen) && Curpage->nextp )
			{
				tpage = Curpage->nextp;
				noffset = 0;
			}
			for( tmark = Mrklist; tmark; tmark = tmark->prev )
				if( tmark->mpage == Curpage && tmark->moffset >= Curchar )
				{
					if( tmark->moffset >= Curchar + quan )
						tmark->moffset -= quan;
					else
					{
						tmark->mpage = tpage;
						tmark->moffset = noffset;
					}
				}
		}
		Makecur( tpage );
		Makeoffset( noffset );
	}
	Vsetmod( TRUE );
}


/* Delete from the point to the Mark. */
void Bdeltomrk(Mark *tmark)
{
	if(Bisaftermrk(tmark))
		Bswappnt(tmark);
	while(Bisbeforemrk(tmark))
		if(Curpage == tmark->mpage)
			Bdelete(tmark->moffset - Curchar);
		else
			Bdelete(Curplen - Curchar);
}


/* Return current screen col of point. */
int Bgetcol( flag, col )
Boolean flag;
int col;
{
	Mark pmark;

	Bmrktopnt(&pmark);
	if(Bcrsearch(NL)) Bmove1();
	while(!Bisatmrk(&pmark) && !Bisend())
	{
		col += Width(Buff(), col, flag);
		Bmove1();
	}
	return(col);
}


/* Insert a character in the current buffer. */
void Binsert(Byte new)
{
	register Mark *btmark;

	if(Curplen == PSIZE && !Pagesplit()) return;
	memmove(Curcptr + 1, Curcptr, Curplen - Curchar);
	*Curcptr++ = new;
	++Curplen;
	++Curchar;
	Curbuff->bmodf = MODIFIED;
	Curmodf = TRUE;

	undo_add(1);

	for(btmark = Mrklist; btmark; btmark = btmark->prev)
		if(btmark->mpage == Curpage && btmark->moffset >= Curchar)
			++(btmark->moffset);
	Vsetmod(FALSE);

}


/* Insert a string into the current buffer. */
void Binstr(str)
char *str;
{
	while(*str)
		Binsert(*str++);
}


/* Returns TRUE if point is after the mark. */
Boolean Bisaftermrk(Mark *tmark)
{
	Page *tp;

	if (!tmark->mpage || tmark->mbuff != Curbuff)
		return FALSE;
	if (tmark->mpage == Curpage)
		return Curchar > tmark->moffset;
	for(tp = Curpage->prevp; tp && tp != tmark->mpage; tp = tp->prevp) ;
	return tp != NULL;
}


/* True if the point precedes the mark. */
Boolean Bisbeforemrk( tmark )
Mark *tmark;
{
	register Page *tp;

	if(!tmark->mpage || tmark->mbuff != Curbuff) return FALSE;
	if(tmark->mpage == Curpage) return Curchar < tmark->moffset;
	for(tp = Curpage->nextp; tp && tp != tmark->mpage; tp = tp->nextp) ;
	return tp != NULL;
}


/* Returns the length of the buffer. */
long Blength(tbuff)
Buffer *tbuff;
{
	register Page *tpage;
	Page *spage;
	register long len;

	Curpage->plen = Curplen;
	spage = Curpage;
	for(len = 0, tpage = tbuff->firstp; tpage; tpage = tpage->nextp)
	{
		if(tpage->lines == EOF) Makecur(tpage);
		len += tpage->plen;
	}
	Makecur(spage);
	return len;
}


/* Return the current position of the point. */
unsigned long Blocation(lines)
unsigned *lines;
{
	unsigned long len;
	Page *tpage, *spage;

	spage = Curpage;
	len = 0l;
	if(lines) *lines = 1;
	for( tpage = Curbuff->firstp; tpage != spage; tpage = tpage->nextp )
	{
		if( tpage->lines == EOF )
		{
			Makecur( tpage );
			tpage->lines = Cntlines( Curplen );
		}
		if(lines) *lines += tpage->lines;
		len += tpage->plen;
	}
	Makecur( spage );
	if(lines) *lines += Cntlines( Curchar );
	return( len + Curchar );
}


/* Number of lines in buffer */
long Blines(Buffer *buff)
{
	unsigned long lines;
	Page *tpage, *spage;

	if(Curmodf) Curpage->lines = EOF;
	spage = Curpage;
	lines = 1;

	for(tpage = buff->firstp; tpage; tpage = tpage->nextp)
	{
		if(tpage->lines == EOF)
		{
			Makecur(tpage);
			tpage->lines = Cntlines(Curplen);
		}
		lines += tpage->lines;
	}

	Makecur(spage);
	return(lines);
}


/*
Try to put Point in a specific column.
Returns actual Point column.
*/
int Bmakecol( col, must )
int col;
Boolean must;
{
	int tcol = 0, wid;

	if( Bcrsearch(NL) ) Bmove1();
	while( tcol < col && !ISNL(Buff()) && !Bisend() )
	{
		tcol += Width( Buff(), tcol, !must );
		Bmove1();
	}
	if( must && tcol < col )
	{
		if( tcol + (wid = Bwidth('\t', tcol)) < col )
			tcol -= Tabsize - wid;
		Tindent( col - tcol );
	}
	return( tcol );
}


/* Move the point relative to its current position.
 *
 * This routine is the most time-consuming routine in the editor.
 * Because of this, it is highly optimized. Makeoffset() calls have
 * been inlined here.
 *
 * Since Bmove(1) is used the most, a special call has been made.
 */
Boolean Bmove1()
{
	if(++Curchar < Curplen)
	{	/* within current page */
		++Curcptr;
		return TRUE;
	}
	if(Curpage->nextp)
	{	/* goto start of next page */
		Makecur(Curpage->nextp);
		Curchar = 0;
		Curcptr = Cpstart;
		return TRUE;
	}
	/* At EOB */
	Makeoffset(Curplen);
	return FALSE;
}

Boolean Bmove(dist)
register int dist;
{
	while(dist)
	{
		dist += Curchar;
		if(dist >= 0 && dist < Curplen)
		{	/* within current page Makeoffset dist */
			Curchar = dist;
			Curcptr = Cpstart + dist;
			return TRUE;
		}
		if(dist < 0)
		{	/* goto previous page */
			if(Curpage == Curbuff->firstp)
			{	/* past start of buffer */
				Makeoffset(0);
				return FALSE;
			}
			Makecur(Curpage->prevp);
			Curchar = Curplen;			/* Makeoffset Curplen */
			Curcptr = Cpstart + Curplen;
		}
		else
		{	/* goto next page */
			if(Curpage == Curbuff->lastp)
			{	/* past end of buffer */
				Makeoffset(Curplen);
				return FALSE;
			}
			dist -= Curplen;			/* must use this Curplen */
			Makecur(Curpage->nextp);
			Curchar = 0;				/* Makeoffset 0 */
			Curcptr = Cpstart;
		}
	}
	return TRUE;
}


/* Put the mark where the point is. */
void Bmrktopnt(tmark)
Mark *tmark;
{
	tmark->mbuff   = Curbuff;
	tmark->mpage   = Curpage;
	tmark->moffset = Curchar;
}


/* Put the current buffer point at the mark */
void Bpnttomrk(tmark)
Mark *tmark;
{
	if(tmark->mpage)
	{
		if(tmark->mbuff != Curbuff)
			Bswitchto(tmark->mbuff);
		Makecur(tmark->mpage);
		Makeoffset(tmark->moffset);
	}
}


void Bempty()
{
	register Mark *btmark;

	Makecur( Curbuff->firstp );
	while( Curpage->nextp )
		Freepage( Curbuff, Curpage->nextp );
	for( btmark = Mrklist; btmark; btmark = btmark->prev )
		if( btmark->mpage && btmark->mbuff == Curbuff )
		{
			btmark->mpage = Curpage;
			btmark->moffset = 0;
			btmark->modf = TRUE;
		}
	Curplen = Curchar = 0;		/* reset to start of page */
	Curcptr = Cpstart;
	Curmodf = TRUE;

	undo_clear(Curbuff);
}


/* Point to off the end of the buffer */
void Bshoveit()
{
	Makecur(Curbuff->lastp);
	Makeoffset(Curplen + 1);
}


/* Swap the point and the mark. */
void Bswappnt(Mark *tmark)
{
	Mark tmp;

	tmp.mbuff	= Curbuff;			/* Point not moved out of its buffer */
	tmp.mpage	= tmark->mpage;
	tmp.moffset	= tmark->moffset;
	Bmrktopnt(tmark);
	Bpnttomrk(&tmp);
}


void Bswitchto(new)
Buffer *new;
{
	if(new && new != Curbuff)
	{
		if(Curbuff)
		{
			Curbuff->pnt_page   = Curpage;
			Curbuff->pnt_offset = Curchar;
		}
		Makecur(new->pnt_page);
		Makeoffset(new->pnt_offset);
		Curbuff = new;
	}
}


/* Set the point to the end of the buffer */
void Btoend()
{
	Makecur(Curbuff->lastp);
	Makeoffset(Curplen);
}


/* Set the point to the start of the buffer. */
void Btostart()
{
	Makecur(Curbuff->firstp);
	Makeoffset(0);
}


#define XBread(fd, blk, addr) read(fd, addr, PSIZE)

/*
Load the file 'fname' into the current buffer.
Returns  0	successfully opened file
		 1  ENOENT		no such file
		-1	EACCESS		file share violation
		-2	EMFILE		out of fds
*/
int Breadfile(fname)
char *fname;
{
	char msg[ PATHMAX + 20 ];
	struct stat sbuf;
	int fd, len, blk;

	if( (fd = open(fname, READ_MODE)) == EOF || fstat(fd, &sbuf) == EOF )
	{
		switch(errno)
		{
		case EACCES:
			sprintf(msg, "No read access: %s", fname);
			Error( msg );
			return( -1 );
		case EMFILE:
			Error( "Out of File Descriptors." );
			return( -2 );
		default:
			return( 1 );
		}
	}

	Curbuff->mtime = sbuf.st_mtime;		/* save the modified time */
	sprintf( msg, "Reading %s", Lastpart(fname) );
	Echo( msg );

	Bempty();
	{
		for( blk = 0; (len = XBread(fd, blk, Cpstart)) > 0; ++blk )
		{
			Curplen = len;
			Makeoffset( 0 );
			Curmodf = TRUE;
			if( !Newpage(Curbuff, Curpage, NULL) )
				break;
			Makecur( Curpage->nextp );
		}
		if( blk > 0 && Curplen == 0 )
			Freepage( Curbuff, Curpage );	/* whoops - alloced 1 too many */
		(void)close( fd );
	}
	Btostart();
	Curbuff->bmodf = FALSE;
	Clrecho();
	return( 0 );
}


/*	Write the current buffer to an open file descriptor.
 *	Returns:	TRUE	if write successfull
 *			FALSE	if write failed
 */
int Bwritefd(int fd)
{
	Mark pmark;				/* no mallocs! */
	Page *tpage;
	struct stat sbuf;
	int status = TRUE;

	Bmrktopnt( &pmark );
	for( tpage = Curbuff->firstp; tpage && status; tpage = tpage->nextp )
	{
		Makecur( tpage );
		status = XBput( fd, Cpstart, Curplen );
	}
	/* flush the buffers */
	if( (status &= XBput(fd, NULL, EOF)) )
	{	/* get the time here - on some machines (SUN) 'time' incorrect */
		fstat( fd, &sbuf );
		Curbuff->mtime = sbuf.st_mtime;
	}
	else
		Error( "Unable to write file." );
	(void)close( fd );
	Bpnttomrk( &pmark );

	if( status )
		Curbuff->bmodf = FALSE;

	return status;
}

/*	Write the current buffer to the file 'fname'.
 *	Handles the backup scheme according to Vars[VBACKUP].val.
 *	Returns:	TRUE	if write successfull
 *				FALSE	if write failed
 *				ABORT	if user didn't want to overwrite
 */
int Bwritefile( fname )
char *fname;
{
	extern int Cmask;
	char bakname[ PATHMAX + 1 ];
	int fd, mode, status = TRUE, bak = FALSE;
	struct stat sbuf;
	int nlink;

	if(!fname) return TRUE;

	/* If the file existed, check to see if it has been modified. */
	if(Curbuff->mtime && stat(fname, &sbuf) == 0)
	{
		if(sbuf.st_mtime > Curbuff->mtime)
		{
			sprintf(PawStr, "WARNING: %s has been modified. Overwrite? ",
				Lastpart(fname));
			if(Ask(PawStr) != YES) return ABORT;
		}
		mode  = sbuf.st_mode;
		nlink = sbuf.st_nlink;
	}
	else
	{
		mode  = Cmask;
		nlink = 1;
	}

	/* check for links and handle backup file */
	Bakname(bakname, fname);
	if(nlink > 1)
	{
		sprintf(PawStr, "WARNING: %s is linked. Preserve? ", Lastpart(fname));
		switch(Ask(PawStr))
		{
			case YES:
				if(Vars[VBACKUP].val)
					bak = Cp(fname, bakname);
				break;
			case NO:
				if(Vars[VBACKUP].val)
					bak = Mv(fname, bakname);
				else
					unlink(fname);	/* break link */
				break;
			case ABORT:
				return ABORT;
		}
	}
	else if(Vars[VBACKUP].val)
		bak = Mv(fname, bakname);

	/* Write the output file */
	if( (fd = open(fname, WRITE_MODE, mode)) != EOF )
		status = Bwritefd(fd);
	else
	{
		if( errno == EACCES )
			Error( "File is read only." );
		else
			Error( "Unable to open file." );
		status = FALSE;
	}

	/* cleanup */
	if( status )
	{
		Clrecho();
	}
	else if(bak)
	{
		if(sbuf.st_nlink)
		{
			Cp(bakname, fname);
			unlink(bakname);
		}
		else
			Mv(bakname, fname);
	}

	return( status );
}


int Cntlines( stop )
int stop;
/* count the lines (NLs) in the current page up to offset 'stop' */
{
	register Byte *p, *n;
	int lines = 0, end;

	for(p = Cpstart, end = stop;
		(n = (Byte *)memchr(p,NL,end)) != NULL;
		++lines, p = n)
	{
		++n;
		end -= n - p;
	}
	return( lines );
}


/* Make the point be dist chars into the page. */
void Makeoffset(dist)
int dist;
{
	Curchar = dist;
	Curcptr = Cpstart + dist;
}


Boolean Mrkaftermrk( mark1, mark2 )
Mark *mark1, *mark2;
/* True if mark1 follows mark2 */
{
	Page *tpage;
	
	if( !mark1->mpage || !mark2->mpage || mark1->mbuff != mark2->mbuff )
		return( FALSE );        /* marks in different buffers */
	if( mark1->mpage == mark2->mpage )
		return( mark1->moffset > mark2->moffset );
	for( tpage = mark1->mpage->prevp; tpage && tpage != mark2->mpage;
			 tpage = tpage->prevp );
	return( tpage != NULL );
}


Boolean Mrkatmrk( mark1, mark2 )
Mark *mark1, *mark2;
/* True if mark1 is at mark2 */
{
	return( mark1->mbuff == mark2->mbuff &&
			mark1->mpage == mark2->mpage &&
			mark1->moffset == mark2->moffset );
}


#ifdef NOT_USED
Boolean Mrkbeforemrk( mark1, mark2 )
Mark *mark1, *mark2;
/* True if mark1 precedes mark2 */
{
	Page *tpage;
	
	if( !mark1->mpage || !mark2->mpage || mark1->mbuff != mark2->mbuff )
		return( FALSE );        /* Marks not in same buffer */
	if( mark1->mpage == mark2->mpage )
		return( mark1->moffset < mark2->moffset );
	for( tpage = mark1->mpage->nextp; tpage && tpage != mark2->mpage;
			 tpage = tpage->nextp );
	return( tpage != NULL );
}
#endif


/* Free up the given mark and remove it from the list.
 * Cannot free a scrnmark!
 */
void Unmark(Mark *mptr)
{
	if (mptr) {
		if (mptr->prev)
			mptr->prev->next = mptr->next;
		if (mptr->next)
			mptr->next->prev = mptr->prev;
		if (mptr == Mrklist)
			Mrklist = mptr->prev;
		free((char *)mptr);
	}
}
