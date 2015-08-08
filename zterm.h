#ifndef _ZTERM_H_
#define _ZTERM_H_

#if TERMINFO
#include <term.h>
#include <curses.h>
#elif TERMCAP
#include <termcap.h>

extern char *cm[];
#else /* ANSI */
#define tlinit()
#define tlfini()
#endif

#if TERMINFO || TERMCAP
#define TPUTS(s)         tputs(s, 1, putchar)

void tlinit(void);
void tlfini(void);
#endif

/* Defined in kbd.c */
extern char *Tkeys[];
extern unsigned Key_mask;

#endif
