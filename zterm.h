#ifndef _ZTERM_H_
#define _ZTERM_H_

#ifdef TERMINFO
#include <term.h>
#include <curses.h>
#elif defined(TERMCAP) || defined(TERMCAP_KEYS)
#include <termcap.h>

extern char *cm[];
#else /* ANSI */
#define tlinit()
#define tlfini()
#endif

#if defined(TERMINFO) || defined(TERMCAP) || defined(TERMCAP_KEYS)
#define TPUTS(s) tputs(s, 1, putchar)

void tlinit(void);
void tlfini(void);
#endif

/* Defined in kbd.c */
extern char *Tkeys[];
extern unsigned Key_mask;

#endif
