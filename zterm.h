#ifndef _ZTERM_H_
#define _ZTERM_H_

#include "tinit.h"

#ifdef TERMINFO
#include <term.h>
#include <curses.h>
#elif defined(TERMCAP) || defined(TERMCAP_KEYS)
#include <termcap.h>
#endif

void tlinit(void);
void tlfini(void);

#if defined(TERMINFO) || defined(TERMCAP) || defined(TERMCAP_KEYS)
#define TPUTS(s) tputs(s, 1, putchar)
#endif

/* Defined in kbd.c */
extern char *Tkeys[];
extern unsigned Key_mask;

#endif
