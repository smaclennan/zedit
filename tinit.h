#ifndef _TINIT_H_
#define _TINIT_H_

/* WARNING: Currently -DTERMINFO or -DTERMCAP* only work for zedit */

#define	ROWMAX			110
#define COLMAX			256

#include "buff.h"

void tinit(void);
void tsize(int *rows, int *cols);

/* These are weak functions that can be overridden by the app */
void tainit(void);
void tafinit(void);
void hangup(int signo);

#ifdef WIN32
#define tflush()
extern HANDLE hstdin, hstdout;	/* Console in and out handles */
#else
#define tflush() fflush(stdout)
#endif

/* Optimized routines for output */
extern int Prow, Pcol;

/** Move the point to row+col but don't move the cursor */
#define tsetpoint(r, c)		(Prow = r, Pcol = c)
void tforce(void);
void tputchar(Byte ch);
void t_goto(int row, int col);
void t_cleol(void);

/** Keyboard input. */
Byte tgetkb(void);

#endif
