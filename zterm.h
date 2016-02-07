#ifndef _ZTERM_H_
#define _ZTERM_H_

#include "tinit.h"

void tlinit(void);
void tlfini(void);

/* Defined in kbd.c */
extern char *Tkeys[];
extern unsigned Key_mask;

#endif
