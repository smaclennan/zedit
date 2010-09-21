/* config.h - Zedit user configurable defines
 * Copyright (C) 1988-2010 Sean MacLennan
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

/* OPERATING SYSTEM - We attempt to autodetect.
 * I have not tested anything other than Linux in a decade, you have
 * been warned!
 */
#ifdef __linux__
# define LINUX
#elif defined(__BSD__)
# define BSD
/* You can also set either of these variations. */
/* # define SUNBSD */
/* # define ULTRIX */
#elif defined(__unix__)
# define SYSV4
/* # define SYSV2 */
#else
# error OS not detected.
#endif

/* SCREEN DRIVER - define only one.
 * XWINDOWS should be set automagically.
 * Unless you are running on an ancient dumb terminal, you probably want ANSI.
 * Linux wants ANSI, trust me on this ;)
 */
#ifndef XWINDOWS
#define ANSI
/* #define TERMINFO */
#endif

/* USER CONFIGURABLE - don't define any, see if I care */
#define DBG		1		/* turn debugs on */
#define SLOW_DISK	0		/* File writes try to buffer up
					 * the data to a block size.
					 */
#define COMMENTBOLD	1		/* bold C comments */
#define UNDO            1		/* EXPERIMENTAL undo code */

#ifndef MINCONFIG
/* Warning: These are ifdefs, you must comment them out to disable them! */
#define HELP				/* Help */
#define SHELL				/* shell interface */
#define SPELL				/* ispell interface */
#define TAGS				/* tag file support */
#endif

/* DON'T TOUCH THESE */
#ifdef LINUX
#define SYSV4
#endif
#ifdef SYSV4
#define SYSV2
#endif
#if defined(HELP) || defined(SHELL) || defined(SPELL) || defined(TAGS)
#define PIPESH
#elif defined(XWINDOWS)
/* X always needs these */
#define PIPESH
#define SHELL
#endif
