/* config.h - Zedit user configurable defines */

#ifndef _CONFIG_H_
#define _CONFIG_H_

/* OPERATING SYSTEM */
#ifdef __unix__
# define HAVE_POLL
# define HAVE_TERMIOS
# define SHELL 1
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
#define UNDO            0		/* EXPERIMENTAL undo code */

#include "configure.h"

/* DON'T TOUCH THESE */
#if ANSI + TERMCAP + TERMINFO != 1
# error "You can't set more than one"
#endif

#endif /* _CONFIG_H_ */
