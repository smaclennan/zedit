#ifndef _TINIT_H_
#define _TINIT_H_

/* WARNING: Currently -DTERMINFO or -DTERMCAP* only work for zedit */

#include "buff.h"

#ifdef TERMINFO
#include <term.h>
#include <curses.h>
#define TPUTS(s) tputs(s, 1, putchar)
#elif defined(TERMCAP) || defined(TERMCAP_KEYS)
#include <termcap.h>
#define TPUTS(s) tputs(s, 1, putchar)
extern char *cm[]; /* Zedit only */
#endif

#define	ROWMAX			110
#define COLMAX			256

void tinit(void);
void tsize(int *rows, int *cols);

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
void tcleol(void);
void tclrwind(void);

/** Keyboard input. */
Byte tgetkb(void);

#endif
