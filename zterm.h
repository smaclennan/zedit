#ifndef _ZTERM_H_
#define _ZTERM_H_

#if TERMINFO
#include <term.h>
#include <curses.h>
#elif TERMCAP
#include <termcap.h>

extern char *cm[];
#elif TERMCAP_KEYS
#include <termcap.h>
#else /* ANSI */
#define tlinit()
#define tlfini()
#endif

#if TERMINFO || TERMCAP || TERMCAP_KEYS
#define TPUTS(s)         tputs(s, 1, putchar)

void tlinit(void);
void tlfini(void);
#endif

/* Defined in kbd.c */
extern char *Tkeys[];
extern unsigned Key_mask;

void dump_key(int keyn, char *key, const char *suffix);

#endif
