#ifndef _TINIT_H_
#define _TINIT_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef TERMCAP
#include <termcap.h>
#define TPUTS(s) tputs(s, 1, putchar)
extern char *cm[];
#endif

#define	ROWMAX			110
#define COLMAX			256

void tinit(void);
void tsize(int *rows, int *cols);

#ifdef WIN32
#include <Windows.h>
#define tflush()
extern HANDLE hstdin, hstdout;	/* Console in and out handles */

#define ATTR_NORMAL	(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY)
#define ATTR_REVERSE	(BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY)
#else
#define tflush() fflush(stdout)
#endif

/* windows.h must be before buff.h... no idea why */
#include "buff.h"

/* Optimized routines for output */
extern int Prow, Pcol;

/** Move the point to row+col but don't move the cursor */
#define tsetpoint(r, c)		(Prow = r, Pcol = c)
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

#endif
