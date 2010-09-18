/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/

/* types */
/* assume ansi compilers can handle void ... */
typedef void Proc;
#define ARGS(x)		x
#define NOARGS		(void)
#define ZPROC(x)	Proc x (void);

#include <string.h>
#include <memory.h>

#ifndef _XtIntrinsic_h
typedef int Boolean;
#endif

typedef unsigned char Byte;
typedef unsigned short Short;
typedef unsigned Word;

/* These are portable across different Unix's */
#define TOLOWER(c)		(isupper(c) ? tolower(c) : c)
#define TOUPPER(c)		(islower(c) ? toupper(c) : c)

/* defines */
#ifndef NULL
#define NULL        0
#endif

#ifndef TRUE
#define FALSE		0
#define TRUE		1
#endif
#define INVALID		-1

#define BUFSIZE 	512

/* The memory usage for screen stuff is approx:  (ROWMAX + 1) x 25 + COLMAX */
/*                               Xwindows adds:	 ROWMAX * 4 + COLMAX * 4    */

/* NOTE: We assume COLMAX >= ROWMAX (xinit.c) */
#define	ROWMAX				110
#define	COLMAX				256

#ifdef sun4
typedef unsigned long ulong;
#endif

void Dbg ARGS((char *fmt, ...));

extern char G_start[], G_end[];
