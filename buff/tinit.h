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

/* Terminal code and how to use it.
 *
 * To use any of the terminal code you first need to call
 * tinit(). This initializes the terminal for raw character at a time
 * input with no echo. If that is all you need, you are done.
 *
 * You can use tsize() to get the current size of the terminal. This
 * is not required for any of the other terminal functions.
 *
 * The main set of optimized functions are:
 *     tputchar(Byte ch);
 *     t_goto(int row, int col);
 *     tcleol(void);
 *     tclrwind(void);
 *     tflush(void);
 *
 * If you stick with these functions you should have no problems since
 * they are designed to work together. There is one caveat; if you
 * want to use ANSI control codes you must use twrite(). This is
 * because tputchar() assumes a character takes up a space, but ANSI
 * control codes do not.
 *
 * But outside of ANSI control codes, do not mix twrite() (or worse
 * _twrite) with the optimized functions.
 *
 * If you want to deal with everything yourself, go ahead and use
 * twrite()/tflush() if you want some buffering and _twrite() if you
 * don't. Again, don't mix twrite() and _twrite() without calling
 * tflush() in between.
 */

/** @defgroup term Terminal functions
 * Functions that initialize the terminal and some optimized insert
 * and movement functions.
 */

/** @addtogroup term
 * @{
*/

#if defined(__unix__)
/* This is user configurable, turn it off if you want */
#define TBUFFERED
#endif

#define	ROWMAX			110
#define COLMAX			256

void tinit(void);
void tsize(int *rows, int *cols);

/* windows.h must be before buff.h... no idea why */
#include "buff.h"

/* Optimized routines for output */
extern int Prow, Pcol;

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
int tputc(int c);
void tflush(void);
#else
/** Write to stdout */
static inline int twrite(const void *buf, int count)
{
	return write(1, buf, count);
}

static inline int tputc(int c)
{
	write(1, &c, 1);
	return c;
}

#define tflush()
#endif

#ifdef TERMCAP
#include <termcap.h>
#define TPUTS(s) tputs(s, 1, tputc)
extern char *cm[];
extern char *termcap_end;
#endif

#ifdef TERMINFO
#include <term.h>
#include <curses.h>
extern char *cm[];
#define TPUTS(s) tputs(s, 1, tputc)
#endif

/* TERMCAP and TERMINFO only */
void set_tkey(int i, char *key);

void st_hack(void);

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
