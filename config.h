/* config.h - Zedit user configurable defines */

#ifndef _CONFIG_H_
#define _CONFIG_H_

/* OPERATING SYSTEM */
#ifdef __unix__
# define HAVE_POLL
# define HAVE_TERMIOS
#else
# error OS not detected.
#endif

/* Set SYSV4 if you need signal handlers to reset themselves. */
/* #define SYSV4 */

/* Define this if you have 16bit ints */
/* #define INT_IS_16BITS */

/* SCREEN DRIVER - define only one.
 * Unless you are running on an ancient dumb terminal, you probably
 * want ANSI. Trust me on this ;)
 */
#define ANSI		1
#define TERMCAP		0
#define TERMINFO	0

/* USER CONFIGURABLE - don't define any, see if I care */
#define MINCONFIG	0		/* Minimal configuration */

#if !MINCONFIG
#define COMMENTBOLD	1		/* bold C/shell comments */
#define HELP		1		/* Help */
#define LIFE		1		/* Game of life */

#define SHELL		1		/* shell interface */
#define SPELL		1		/* ispell interface */
#define TAGS		0		/* tag file support */

#define UNDO            0		/* EXPERIMENTAL undo code */
#endif

#include "configure.h"

/* DON'T TOUCH THESE */
#if SHELL || TAGS || SPELL
#undef  SHELL
#define SHELL 1
#define PIPESH 1
#endif
#if ANSI + TERMCAP + TERMINFO > 1
# error "You can't set more than one"
#endif
#if TERMCAP
#undef COMMENTBOLD
#endif

#endif /* _CONFIG_H_ */
