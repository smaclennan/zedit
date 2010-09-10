/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"


#ifdef CACHED
/*
 *		DISK CACHING ROUTINES
 *
 */

/* cache buffer globals */
#define NBUF			9			/* number of block buffers in cache */
#define MINRDBUF 		(NBUF / 3)	/* cache flush threshold */

/* buffer header bit flags */
#define XBFREE          0
#define XBREAD			1
#define XBWRITE			2

/* cache buffer structure */
typedef struct _xbuf
{
	int xbflags;					/* buffer flags */
	int xbfd;						/* file which buf points to */
	unsigned xblk;					/* blk offset in file */
	unsigned xbfreq;				/* param for frequency of use algo */
	unsigned xbage;					/* param for LRU algo */
	Byte buf[PSIZE];				/* buffer proper */
	unsigned xblen;					/* len of buffer */
	struct _xbuf *next;				/* next buffer on free list */
} CACHE;

/* Some versions of Unix don't define this... */
#ifndef EDQUOT
#define EDQUOT 511
#endif

/* prototypes for local XB functions */
static CACHE *XBalloc ARGS((void));
static CACHE *XBfindblk ARGS((int blk, int fd));
static void XBfree ARGS((CACHE *buf));
static int XBio ARGS((int fd, int rwfn, int blk, Byte *addr, int len));
static CACHE *XBnextwrite ARGS((void));

unsigned ioage = 0;						/* counter of calls to I/O func */
CACHE	bufpool[ NBUF ];			/* pool of buffers */
CACHE *freebuf;					/* free buffer list ptr */


/* Initalization of cache buffers. */
void XBinit()
{
	unsigned i;
	
	freebuf = NULL;
	for(i = 0; i < NBUF; ++i) XBfree(&bufpool[i]);
}


/* Clear out all the read buffers associated with 'fd'.
 * This is necessary in between Breadfile calls to prevent "matches".
 */
void XBclear(fd)
register int fd;
{
	int i;

	for(i = 0; i < NBUF; ++i)
		if(bufpool[i].xbfd == fd && bufpool[i].xbflags == XBREAD)
			XBfree(&bufpool[i]);
}


/* High level write for non-block writes.
 * Buffers up the input until a PSIZE block is reached,
 * then sends it to XBwrite.
 * Can only be used on ONE file at a time!
 * A 'XBput( fd, NULL, EOF )' call should be made before closing the file.
 * For MSDOS, converts LF to CR LF.
 * NOTE: Special code for Zedit added for AddNL
 */
Boolean XBput(fd, addr, len)
int fd;
Byte *addr;
unsigned len;
{
	static Byte buf[PSIZE];
	static int blk = 0, buflen = 0, lastch = 0;
#if MSDOS
	static Boolean didcr = FALSE;
#endif
	int wrlen, nwrt, rc = TRUE;

	if(len == 0) return TRUE;
	if(len == EOF)
	{	/* flush the buffer and reset */
		if(Vars[VADDNL].val)
		{
			if(buflen > 0) lastch = buf[buflen - 1];
			if(lastch != '\n') XBput(fd, (Byte *)"\n", 1);
		}
		rc = XBio(fd, XBWRITE, blk, buf, buflen) == buflen;
		if(!XBflush(&nwrt))	/* flush all write buffers */
			rc = FALSE;
		buflen = blk = lastch = 0;
	}
	else
#if !MSDOS
	{
		wrlen = (buflen + len > PSIZE) ? (PSIZE - buflen) : len;
		memcpy(&buf[buflen], addr, wrlen);
		buflen += wrlen;
	}
#else
	{	/* must convert \n to \r\n */
		wrlen = 0;
		if(didcr)
		{
			buf[buflen++] = addr[wrlen++];
			didcr = FALSE;
		}
		while(buflen < PSIZE && wrlen < len)
			if(addr[wrlen] == NL)
			{
				buf[buflen++] = '\r';
				if(buflen < PSIZE)
					buf[buflen++] = addr[wrlen++];
				else
					didcr = TRUE;
			}
			else
				buf[buflen++] = addr[wrlen++];
	}
#endif
	if(buflen == PSIZE)
	{
		rc = XBio(fd, XBWRITE, blk, buf, PSIZE) == PSIZE;
		lastch = buf[PSIZE - 1];
		buflen = 0;
		++blk;
		rc &= XBput(fd, &addr[wrlen], len - wrlen);
	}
	return rc;
}


/* Medium level cache read from an open file. */
int XBread(fd, blk, addr)
int fd, blk;
Byte *addr;
{
	return(XBio(fd, XBREAD, blk, addr, PSIZE));
}


/* Medium level cache write to an open file. */
int XBwrite(fd, blk, addr)
int fd, blk;
Byte *addr;
{
	return(XBio(fd, XBWRITE, blk, addr, PSIZE));
}


/*
Flush all pending writes in the buffer pool and convert write status
to read.
Returns success/failure. The number of buffers changed returned in *nwrt.
Writing of buffers is done to minimize head movement.
*/
Boolean XBflush(nwrt)
int *nwrt;
{
	CACHE *ip;
	
	for(*nwrt = 0; ip = XBnextwrite(); ++*nwrt)
	{
		if(lseek(ip->xbfd, (long)(ip->xblk) * (long)PSIZE, 0) == EOF ||
			write(ip->xbfd, (char *)ip->buf, ip->xblen) == EOF)
				return FALSE;
		ip->xbflags = XBREAD;
	}
	return TRUE;
}


/* LOCAL functions */


/* Return a buffer to the pool. */
static void XBfree( buf )
CACHE *buf;
{
	buf->xbflags = XBFREE;      /* clear all flags */
	buf->xbfd = EOF;
	buf->next = freebuf;        /* link onto free list */
	freebuf = buf;
}


/*
Allocate a buffer from the pool.
Look for the buffer with the lowest freq of use.
If more than one with the lowest freq of use,
choose the (LRU) one.
If the number of read-type buffers is below the minimum,
flush any  buffers.
*/
static CACHE *XBalloc()
{
	int nflush, nrdbuf;
	unsigned freq, i, oldest, age;
	register CACHE *ip;

	if( freebuf )
	{
		ip = freebuf;
		freebuf = freebuf->next;
		memset( ip->buf, 0, PSIZE );
		return( ip );
	}
	nrdbuf = 0;			/* counter of READ type buffers */
	ip = NULL;			/* ip will be non-NULL if READ buffer found */
	freq = 0x7fff;		/* hunt for XBFREQ less than this (a big number) */
	oldest = 0;			/* hunt for buffer older than this */
	for( i = 0; i < NBUF; ++i )
	{
		if( bufpool[i].xbflags & XBREAD )
		{	/* a READ type buffer */
			++nrdbuf;
			if( bufpool[i].xbfreq <= freq )
			{
				freq = bufpool[i].xbfreq;
				age = ioage - bufpool[i].xbage;
				if( age > oldest )
				{
					oldest = age;
					ip = &bufpool[i];
				}
			}
		}
	}
	if( nrdbuf < MINRDBUF )
		if( !XBflush(&nflush) )
#if MSDOS
			Echo( "Out of Disk Space" );
#else
			switch( errno )
			{
				case EIO:		Echo( "I/O Error" ); break;

				case EDQUOT:
				case ENOSPC:
				case EFBIG:		Echo( "Out of Disk Space" ); break;
				
				default:		Echo( "Bad Cache Write" ); break;
			};
#endif			
	if( ip )
	{
		XBfree( ip );
		return( XBalloc() );
	}
	else if( nflush )
		return( XBalloc() );
	return( NULL );
}


/*
Low level buffered block I/O function.
Give: fd, read/write, desired block, address of data.
Returns number of bytes transfered.
*/
static int XBio( fd, rwfn, blk, addr, len )
int fd, rwfn, blk, len;
Byte *addr;
{
	int i, rdlen;
	CACHE *ip;

	if( (++ioage & 017) == 0 )
		/* every 16 times decay all XBFREQ values by 1/2 */
		for( i = 0; i < NBUF; ++i )
			if( bufpool[i].xbflags & XBREAD )
				/* bufpool[ i ].xbfreq >>= 1;		DY-4 dosen't like this! */
				bufpool[ i ].xbfreq = bufpool[ i ].xbfreq >> 1;
	
	if( ip = XBfindblk(blk, fd) )
	{	/* 
			this block is in a cache buffer.
			increase XBFREQ and reset XBAGE.
		*/
		ip->xbfreq += 128;
		ip->xbage = ioage;
		if( rwfn == XBREAD )
		{
			memcpy( addr, ip->buf, ip->xblen );
			return( ip->xblen );
		}
		else
		{
			memcpy( ip->buf, addr, len );
			ip->xblen = len;
			ip->xbflags = XBWRITE;
			return( len );
		}
	}
	/*
		this block is not in the pool.
		allocate a buffer and read the block
		and do the requested transfer.
	*/
	if( !(ip = XBalloc()) ) return( 0 );
	ip->xbfd = fd;				/* setup header */
	ip->xblk = blk;
	ip->xbage = ioage;
	ip->xbfreq = 0;
	rdlen = EOF;
	if( rwfn == XBWRITE )
	{
		memcpy( ip->buf, addr, len );
		ip->xblen = len;
		ip->xbflags = XBWRITE;
		return( len );
	}
	else if( lseek(ip->xbfd, (long)(ip->xblk) * (long)PSIZE, 0) == EOF ||
			 (rdlen = read(ip->xbfd, (char *)ip->buf, len)) <= 0 )
	{
		if( rdlen < 0 ) Echo( "Bad Cache Read" );
		XBfree( ip );
		return( rdlen );
	}
	else
	{
		ip->xbflags = XBREAD;
		ip->xblen = rdlen;
		return( XBio(fd, rwfn, blk, addr, rdlen) );
	}
}


/* If a block is in the buffer pool, return the buffer */
static CACHE *XBfindblk( blk, fd )
int blk, fd;
{
	register int i;
	register CACHE *ip;
	
	for( i = 0, ip = bufpool; i < NBUF; ++i, ++ip )
		if( ip->xbflags && ip->xblk == blk && ip->xbfd == fd )
				return( ip );
	return( NULL );
}


/*
Scan the buffer pool for buffers marked as WRITE
and return the address of the one with greatest ID
and lowest block number.
This increases speed by about 1/3!!! for a exe increase of 112 bytes!
*/
static CACHE *XBnextwrite()
{
	register int i;
	int fdmax, blkmin;
	CACHE *rval;
	register CACHE *ip;
	
	fdmax = 0;				/* a small, impossible ID number */
    blkmin = 0x7fff;        /* the biggest block number */
	rval = NULL;			/* the return value */
	for( i = 0, ip = bufpool; i < NBUF; ++i, ++ip )
		if( ip->xbflags & XBWRITE )
			if( fdmax < ip->xbfd )
			{
				fdmax = ip->xbfd;
				blkmin = ip->xblk;
				rval = ip;
			}
			else if( fdmax == ip->xbfd && blkmin > ip->xblk )
			{
				blkmin = ip->xblk;
				rval = ip;
			}
	return( rval );
}



#ifdef FAST
/*
This is the fast file read for machines with S L O W drives.
Instead of reading the file into the swap file, it marks the positions
in the input file, which is kept open.
The variable FastSize sets the file size at which this will be run.
This has the drawback that the input file MUST be kept open.
*/
#define FLIP
#ifdef FLIP
static char flip[] = "|/-\\";
#endif

Boolean Fastread( fd, size )
int fd;
unsigned long size;
{
	extern PIM *Pages;
	int i, pcnt;
#ifdef FLIP
	int f = 0;
#endif
	
	/* the first page is already in memory - so fill it in! */
	i = read( fd, Cpstart, PSIZE );
	Curpage->plen = i;
	Pages[ Curpage->apage ].pim_modf = TRUE;

	/* now point the pages into the file */
	pcnt = size / PSIZE + (size % PSIZE != 0);
	for( i = 1; i < pcnt && (Curpage = Newpage(Curbuff, Curpage, NULL)); ++i )
	{
		Curpage->afile = INFILE;
		Curpage->fd = fd;
		Curpage->fpage = i;
		Curpage->plen = PSIZE;
#ifdef FLIP
		if(i % 100 == 0)
		{
			Tputchar(flip[f]);
			Tflush();
#ifdef PC
			--Pcol;
#else
			Tputchar('\b');
			Tflush();
#endif
			f = ++f % 4;
		}
#endif
	}
	if( Curpage && (size = size % PSIZE) )
		Curpage->plen = size;

	/* reset Curpage */
	Curpage = NULL;
	Makecur( Curbuff->firstp );
	Makeoffset( 0 );

	/* set the buffer to Fastread */
	Curbuff->bmode |= FASTREAD;
	
	return( i < pcnt );
}


/* Whenever we write a Fastread buffer we must rebuild the pages */
void Refast()
{
	extern int Fbuffs;

	unsigned long ploc, mloc;
	int junk;
	
	if( Curbuff->firstp->nextp )
	{
		(void)close( Curbuff->firstp->nextp->fd );
		--Fbuffs;
		ploc = Blocation( &junk );
		Bswappnt( Curbuff->mark );
		mloc = Blocation( &junk );
		Breadfile( Curbuff->fname );
		Boffset( mloc );
		Bswappnt( Curbuff->mark );
		Boffset( ploc );
	}
}
#endif

#else

/* NO-CACHE VERSION.
 * These routines assume blocks read/written in order!
 * Can not be used with VBUFF.
 * Can not be used with MSDOS
 */

void XBinit() {}

void XBclear(fd) int fd; {}


int XBread(fd, blk, addr)
int fd, blk;
Byte *addr;
{
	return read(fd, addr, PSIZE);
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

#endif
