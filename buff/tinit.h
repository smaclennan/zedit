/* tinit.h - term definitions
 * Copyright (C) 1988-2017 Sean MacLennan <seanm@seanm.ca>
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

#ifndef _TINIT_H_
#define _TINIT_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/** @defgroup term Terminal functions
 * Functions that initialize the terminal and some optimized insert
 * and movement functions.
 */

/** @addtogroup term
 * @{
*/

#ifdef TERMCAP
#include <termcap.h>
#define TPUTS(s) tputs(s, 1, putchar)
extern char *cm[];
extern char *termcap_end;
#endif

#define	ROWMAX			110
#define COLMAX			256

void tinit(void);
void tsize(int *rows, int *cols);

#ifdef WIN32
#include <Windows.h>
extern HANDLE hstdout;	/* Console out handle */

void tkbdinit(void);

#define ATTR_NORMAL	(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY)
#define ATTR_REVERSE	(BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY)
#endif

#if defined(__unix__) && !defined(TERMCAP)
/* This is user configurable, turn it off if you want */
#define TBUFFERED
#endif

/* windows.h must be before buff.h... no idea why */
#include "buff.h"

/* Optimized routines for output */
extern int Prow, Pcol;

/** Move the point to row+col but don't move the cursor */
void tforce(void);
void tputchar(Byte ch);
void t_goto(int row, int col);
void tcleol(void);
void tclrwind(void);

/* tstyle() attributes */
#define T_NORMAL		0
#define T_BOLD			1
#define T_REVERSE		7
#define T_FG			30
#define T_BG			40
/* Colors should be added to T_FG or T_BG */
#define T_BLACK			 0
#define T_RED			 1
#define T_GREEN			 2
#define T_YELLOW		 3
#define T_BLUE			 4
#define T_MAGENTA		 5
#define T_CYAN			 6
#define T_WHITE			 7

#ifdef TERMCAP
#define T_BELL			256
#else
#define T_BELL			(T_BG | T_RED)
#endif

void tstyle(int style);

/** Low level twrite mainly to get around write return warnings.
 * @param fd File descriptor to write to.
 * @param buf The output buffer.
 * @param count The number of bytes to output.
 * @return Return from write().
 */
static inline int _twrite(int fd, const void *buf, int count)
{
	return write(fd, buf, count);
}

#ifdef TBUFFERED
int twrite(const void *buf, int count);
void tflush(void);
#else
/** Write to stdout */
static inline int twrite(const void *buf, int count)
{
	return write(1, buf, count);
}

#define tflush()
#endif

/** Write string to stderr.
 * @param str String to write.
 * @return Output from _twrite().
 */
static inline int terror(const char *str)
{
	return _twrite(2, str, strlen(str));
}
/* @} */

#endif
