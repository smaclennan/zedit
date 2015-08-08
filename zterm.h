#ifndef _ZTERM_H_
#define _ZTERM_H_

#if TERMINFO
#include <term.h>
#include <curses.h>
#endif

#if TERMCAP
#include <termcap.h>

extern char *cm[];
#endif

/* This is not used in ANSI, but should be safe to define */
#define TPUTS(s)         tputs(s, 1, putchar)

/* Defined in kbd.c */
extern char *Tkeys[];
extern unsigned Key_mask;

#ifdef ANSI
static inline void tlinit(void) {}
static inline void tlfini(void) {}
#endif

#endif
