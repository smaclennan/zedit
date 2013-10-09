/* config.h - Zedit user configurable defines
 * Copyright (C) 1988-2013 Sean MacLennan
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this project; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

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

#define CONFIGDIR "/usr/share/zedit"

/* USER CONFIGURABLE - don't define any, see if I care */
#define MINCONFIG	0		/* Minimal configuration */

#if !MINCONFIG
#define COMMENTBOLD	1		/* bold C comments */
#define WANT_CPPS       0		/* also bold C preprocessor lines */
#define UNDO            0		/* EXPERIMENTAL undo code */
#define HELP		0		/* Help */
#define LIFE		1		/* Game of life */

#define SHELL		1		/* shell interface */
#define SPELL		1		/* ispell interface */
#define TAGS		1		/* tag file support */
#endif

#include "configure.h"

/* DON'T TOUCH THESE */
#if SHELL || TAGS || SPELL
#undef  SHELL
#define SHELL 1
#define PIPESH 1
#endif
#if TERMCAP
#undef COMMENTBOLD
#if ANSI || TERMINFO
#error "You can't more than one"
#endif
#endif
#if ANSI && TERMINFO
#error "You can't set more than one"
#endif

#endif /* _CONFIG_H_ */
